//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_TILE_HUB_H
#define CPPMAIN_TILE_HUB_H

#include "../../message/message.h"

class MemoryAccessHandler;

class TileHub {
public:
    explicit TileHub(MemoryAccessHandler *mah);
    virtual void tick();
    
    inline int get_num_src(eid_t tid) const; // forward
    inline int get_num_edge(eid_t tid) const; // forward
    inline int get_num_dst(vid_t pid) const; // forward
    
    inline vid_t get_new_tile_pid() const;
    inline tid_coalesced assign_tile();
    
    inline void release_tiles(const tid_coalesced &tids);
    inline void release_partition(vid_t pid_old);
    
    inline Tile_T *get_tile_by_tid(eid_t tid) const;
    inline tile_coalesced get_tiles_by_tids(const tid_coalesced &tid) const;
    inline bool check_state_all_end() const;
    inline bool check_last_tile_present() const;

protected:
    MemoryAccessHandler *mah;
    bool is_loading = false;
    
    enum class State : int {
        READY, LOADING, END, TILING
    };
    
    int num_tile_onchip;
    vector<State> p_state;
    vector<Tile_T *> p_data;
    deque<int> q_pi_empty;
    deque<int> q_pi_partitions;
    deque<vector<int>> tile_to_assign;
    long tile_to_assign_elements = 0;
    eid_t tile_to_load_id = 0;
    eid_t tile_loaded_cnt = 0;
    int num_tiles_ready = 0;
    
    void issue_to_mah(int i);
    inline callback_t get_function_callback(int);
    inline int find_position_by_tid(eid_t tid) const;
    inline int find_position_by_pid(vid_t pid) const;
    
};

/*************************************/
/********** LOAD NEW TILE ************/
/*************************************/

inline void TileHub::tick() {
    if (!q_pi_empty.empty() && !is_loading) {
        auto i = q_pi_empty.front();
        q_pi_empty.pop_front();
        is_loading = true;
        issue_to_mah(i);
    }
}

inline callback_t TileHub::get_function_callback(int i) {
    assert(Config::name_arch == EArch::base);
    return [i, this](callback_args_t &msg) {
        is_loading = false;
        assert(this->p_state[i] == TileHub::State::LOADING);
        if (this->p_data[i] != nullptr) { delete this->p_data[i]; }
        this->p_data[i] = msg.tile;
        
        if (msg.tile == nullptr) {
            assert(msg.tag.at("tile") == -1);
            this->p_state[i] = TileHub::State::END;
        } else {
            assert(msg.tag.at("tile") == 1);
            this->p_state[i] = TileHub::State::READY;
            this->tile_loaded_cnt++;
            this->num_tiles_ready++;
            
            if (Config::coalesce) {
                assert(!tile_to_assign.empty());
                long tile_element =
                        msg.tile->forward_edge_total * Config::num_element_feat_edge
                        + msg.tile->source_vertex_total * Config::num_element_feat_src;
                
                if (tile_to_assign.back().empty()) {
                    tile_to_assign.back().push_back(i);
                    tile_to_assign_elements = tile_element;
                    assert(tile_to_assign_elements <= Config::num_total_element_tile);
                } else if (tile_element + tile_to_assign_elements > Config::num_total_element_tile) {
                    tile_to_assign.push_back({i});
                    tile_to_assign_elements = tile_element;
                } else {
                    tile_to_assign.back().push_back(i);
                    tile_to_assign_elements += tile_element;
                }
                
                if (msg.tile->output
                    || (num_tile_onchip - num_tiles_ready == 0
                        && tile_to_assign.size() < 2)) {
                    tile_to_assign.emplace_back();
                    tile_to_assign_elements = 0;
                }
                
            } else { // disable tile-coalesce
                assert(!tile_to_assign.empty());
                assert(tile_to_assign.back().empty());
                tile_to_assign.back().push_back(i);
                tile_to_assign.emplace_back();
            }
        }
    };
}

/*******************************/
/********** RELEASE ************/
/*******************************/

inline void TileHub::release_tiles(const tid_coalesced &tids) {
    for (int ii: tids) {
        int i = find_position_by_tid(ii);
        assert(p_state[i] == State::READY);
        if (!p_data[i]->output) {
            delete p_data[i];
            p_data[i] = nullptr;
            q_pi_empty.push_back(i);
            num_tiles_ready--;
        } else q_pi_partitions.push_back(i);
    }
}

inline void TileHub::release_partition(vid_t pid_old) {
    auto i = q_pi_partitions.front();
    assert(pid_old == p_data[i]->target_vertex_partition_id);
    assert(p_data[i]->output);
    q_pi_partitions.pop_front();
    q_pi_empty.push_back(i);
    num_tiles_ready--;
    delete p_data[i];
    p_data[i] = nullptr;
}

/***************************************/
/********** ASSIGN NEW DATA ************/
/***************************************/

inline vid_t TileHub::get_new_tile_pid() const {
    if (tile_to_assign[0].empty()) return -1;
    return p_data[tile_to_assign[0][0]]->target_vertex_partition_id;
}

inline tid_coalesced TileHub::assign_tile() {
    if (tile_to_assign.size() < 2) return {-1};
    
    tid_coalesced tids(tile_to_assign[0].size());
    for (int ii = 0; ii < tile_to_assign[0].size(); ii++) {
        tids[ii] = p_data[tile_to_assign[0][ii]]->tile_id;
    }
    tile_to_assign.pop_front();
    return tids;
}

/*************************************/
/********** FIND POSITION ************/
/*************************************/

inline int TileHub::find_position_by_tid(eid_t tid) const {
    for (int i = 0; i < num_tile_onchip; i++) {
        if (p_data[i] == nullptr) continue;
        if (tid == p_data[i]->tile_id) return i;
    }
    throw runtime_error("no_such_tile\n");
}

inline int TileHub::find_position_by_pid(vid_t pid) const {
    for (int i = 0; i < num_tile_onchip; i++) {
        if (p_data[i] == nullptr) continue;
        if (pid == p_data[i]->target_vertex_partition_id) return i;
    }
    throw runtime_error("no_such_partition\n");
}

inline int TileHub::get_num_src(eid_t tid) const {
    int i = find_position_by_tid(tid);
    assert(p_state[i] != State::END);
    return p_data[i]->source_vertex_total;
}

inline int TileHub::get_num_dst(vid_t pid) const {
    int i = find_position_by_pid(pid);
    assert(p_state[i] != State::END);
    return p_data[i]->source_vertex_total;
}

inline int TileHub::get_num_edge(eid_t tid) const {
    int i = find_position_by_tid(tid);
    assert(p_state[i] != State::END);
    return p_data[i]->forward_edge_total;
}

inline Tile_T *TileHub::get_tile_by_tid(eid_t tid) const {
    auto i = find_position_by_tid(tid);
    return p_data[i];
}

inline tile_coalesced TileHub::get_tiles_by_tids(const tid_coalesced &tids) const {
    tile_coalesced tiles;
    for (int tid: tids) {
        auto i = find_position_by_tid(tid);
        tiles.push_back(p_data[i]);
    }
    return tiles;
}


/*************************************/
/*********** Check State *************/
/*************************************/

inline bool TileHub::check_state_all_end() const {
    for (const auto &s: p_state)
        if (s != State::END) return false;
    return true;
}

inline bool TileHub::check_last_tile_present() const {
    return !q_pi_partitions.empty();
}

#endif //CPPMAIN_TILE_HUB_H
