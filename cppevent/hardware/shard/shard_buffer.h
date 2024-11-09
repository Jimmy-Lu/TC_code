//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_SHARD_BUFFER_H
#define CPPMAIN_SHARD_BUFFER_H

#include "../message/message.h"

class MemoryAccessHandler;

class ShardHub {
public:
    explicit ShardHub(MemoryAccessHandler *mah);
    virtual void tick();
    
    inline int get_num_src(eid_t sid) const; // forward
    inline int get_num_edge(eid_t sid) const; // forward
    inline int get_num_dst(vid_t iid) const; // forward
    
    inline vid_t get_new_id_interval() const;
    inline sid_coalesced get_new_shards();
    inline void pop_new_shards();
    
    inline void release_shard(const sid_coalesced &sids);
    inline void release_interval(vid_t pid_old);
    
    inline Shard_T *get_shard_by_id(eid_t sid) const;
    inline shard_coalesced get_shards_by_ids(const sid_coalesced &sids) const;
    inline bool check_state_all_end() const;

protected:
    MemoryAccessHandler *mah;
    bool is_loading = false;
    
    enum class State : int {
        READY, LOADING, END
    };
    
    int num_shard_onchip;
    vector<State> p_state;
    vector<Shard_T *> p_data;
    deque<int> queue_pi_empty;
    deque<int> queue_pi_intervals;
    deque<vector<int>> shard_to_assign;
    long shard_to_assign_elements = 0;
    eid_t shard_id_to_load = 0;
    eid_t cnt_shard_loaded = 0;
    int num_shards_ready = 0;
    
    void issue_to_mah(int i);
    inline callback_t get_function_callback(int);
    inline int find_position_by_sid(id_shard_t sid) const;
    inline int find_position_by_iid(id_interval_t iid) const;
    
};

/*************************************/
/********** LOAD NEW SHARD ************/
/*************************************/

inline void ShardHub::tick() {
    if (!queue_pi_empty.empty() && !is_loading) {
        auto i = queue_pi_empty.front();
        queue_pi_empty.pop_front();
        is_loading = true;
        issue_to_mah(i);
    }
}

inline callback_t ShardHub::get_function_callback(int i) {
    assert(Config::name_arch == EArch::base);
    return [i, this](callback_args_t &msg) {
        is_loading = false;
        assert(this->p_state[i] == ShardHub::State::LOADING);
        if (this->p_data[i] != nullptr) { delete this->p_data[i]; }
        this->p_data[i] = msg.shard;
        
        if (msg.shard == nullptr) {
            assert(msg.tag.at("shard") == -1);
            this->p_state[i] = ShardHub::State::END;
        } else {
            assert(msg.tag.at("shard") == 1);
            this->p_state[i] = ShardHub::State::READY;
            this->cnt_shard_loaded++;
            this->num_shards_ready++;
            
            if (Config::coalesce) {
                assert(!shard_to_assign.empty());
                long shard_element =
                        msg.shard->forward_edge_total * Config::num_element_feat_edge
                        + msg.shard->source_vertex_total * Config::num_element_feat_src;
                
                if (shard_to_assign.back().empty()) {
                    shard_to_assign.back().push_back(i);
                    shard_to_assign_elements = shard_element;
                    assert(shard_to_assign_elements <= Config::shard_max_element);
                } else if (shard_element + shard_to_assign_elements > Config::shard_max_element) {
                    shard_to_assign.push_back({i});
                    shard_to_assign_elements = shard_element;
                } else {
                    shard_to_assign.back().push_back(i);
                    shard_to_assign_elements += shard_element;
                }
                
                if (msg.shard->output
                    || (num_shard_onchip - num_shards_ready == 0
                        && shard_to_assign.size() < 2)) {
                    shard_to_assign.emplace_back();
                    shard_to_assign_elements = 0;
                }
                
            } else { // disable shard-coalesce
                assert(!shard_to_assign.empty());
                assert(shard_to_assign.back().empty());
                shard_to_assign.back().push_back(i);
                shard_to_assign.emplace_back();
            }
        }
    };
}

/*******************************/
/********** RELEASE ************/
/*******************************/

inline void ShardHub::release_shard(const sid_coalesced &sids) {
    for (int sid: sids) {
        int i = find_position_by_sid(sid);
        assert(p_state[i] == State::READY);
        if (!p_data[i]->output) {
            delete p_data[i];
            p_data[i] = nullptr;
            queue_pi_empty.push_back(i);
            num_shards_ready--;
        } else queue_pi_intervals.push_back(i);
    }
}

inline void ShardHub::release_interval(vid_t pid_old) {
    auto i = queue_pi_intervals.front();
    assert(pid_old == p_data[i]->target_interval_id);
    assert(p_data[i]->output);
    queue_pi_intervals.pop_front();
    queue_pi_empty.push_back(i);
    num_shards_ready--;
    delete p_data[i];
    p_data[i] = nullptr;
}

/***************************************/
/********** ASSIGN NEW DATA ************/
/***************************************/

inline vid_t ShardHub::get_new_id_interval() const {
    if (shard_to_assign[0].empty()) return -1;
    return p_data[shard_to_assign[0][0]]->target_interval_id;
}

inline void ShardHub::pop_new_shards() {
    assert(!(shard_to_assign.size() < 2));
    shard_to_assign.pop_front();
}

inline sid_coalesced ShardHub::get_new_shards() {
    if (shard_to_assign.size() < 2) return {-1};
    
    sid_coalesced sids(shard_to_assign[0].size());
    for (int ii = 0; ii < shard_to_assign[0].size(); ii++) {
        sids[ii] = p_data[shard_to_assign[0][ii]]->shard_id;
    }
    return sids;
}

/*************************************/
/********** FIND POSITION ************/
/*************************************/

inline int ShardHub::find_position_by_sid(id_shard_t sid) const {
    for (int i = 0; i < num_shard_onchip; i++) {
        if (p_data[i] == nullptr) continue;
        if (sid == p_data[i]->shard_id) return i;
    }
    throw runtime_error("no_such_shard\n");
}

inline int ShardHub::find_position_by_iid(id_interval_t iid) const {
    for (int i = 0; i < num_shard_onchip; i++) {
        if (p_data[i] == nullptr) continue;
        if (iid == p_data[i]->target_interval_id) return i;
    }
    throw runtime_error("no_such_partition\n");
}

inline int ShardHub::get_num_src(id_shard_t sid) const {
    int i = find_position_by_sid(sid);
    assert(p_state[i] != State::END);
    return p_data[i]->source_vertex_total;
}

inline int ShardHub::get_num_dst(id_interval_t iid) const {
    int i = find_position_by_iid(iid);
    assert(p_state[i] != State::END);
    return p_data[i]->source_vertex_total;
}

inline int ShardHub::get_num_edge(id_shard_t sid) const {
    int i = find_position_by_sid(sid);
    assert(p_state[i] != State::END);
    return p_data[i]->forward_edge_total;
}

inline Shard_T *ShardHub::get_shard_by_id(id_shard_t sid) const {
    auto i = find_position_by_sid(sid);
    return p_data[i];
}

inline shard_coalesced ShardHub::get_shards_by_ids(const sid_coalesced &sids) const {
    shard_coalesced shards;
    for (int sid: sids) {
        auto i = find_position_by_sid(sid);
        shards.push_back(p_data[i]);
    }
    return shards;
}


/*************************************/
/*********** Check ThreadState *************/
/*************************************/

inline bool ShardHub::check_state_all_end() const {
    for (const auto &s: p_state)
        if (s != State::END) return false;
    return true;
}

#endif //CPPMAIN_SHARD_BUFFER_H
