//
// Created by Flowerbeach on 2021/2/12.
//
#include "ctrl_top.h"

#include "../compute/compute.h"
#include "../memory/mem_top.h"

using namespace std;

namespace control {
    CtrlTop::CtrlTop(MemoryAccessHandler *mah, ShardHub *th,
                     const vector<ComputeUnit *> &vus,
                     const vector<ComputeUnit *> &mus) : sh(th) {
        decoder = new control::Decode(this, th);
        
        e_state.assign(Config::num_thread, ThreadState::sleep);
        e_id_shard.assign(Config::num_thread, {});
        e_program_cnt.assign(Config::num_thread, 0);
        
        v_state = ThreadState::free;
        v_id_interval = -123;
        v_program_cnt = 0;
        
        num_shard_total = mah->get_num_shard();
        num_interval_total = mah->get_num_interval();
    }
    
    CtrlTop::~CtrlTop() { delete decoder; }
    
    void CtrlTop::tick_scatter() {
        switch (v_state) {
            vid_t id_interval_new;
            
            case ThreadState::free:
                id_interval_new = sh->get_new_id_interval();
                if (id_interval_new < 0) break; // shard not ready
                
                assert(id_interval_new != v_id_interval);
                assert(id_interval_new >= 0);
                v_id_interval = id_interval_new;
                v_state = ThreadState::ready;
                v_program_cnt = 0;
                break;
            
            case ThreadState::ready:
                if (v_program_cnt < programs.at(ProgStage::scatter).size()) {
                    string instruction = programs.at(ProgStage::apply)[v_program_cnt++];
                    auto callback = [this]() {
                        assert(this->v_state == ThreadState::issued);
                        this->v_state = ThreadState::ready;
                    };
                    auto *inst = new Inst(instruction, {-1}, v_id_interval,
                                          Config::num_thread, callback);
                    queue_decode.push_back(inst);
                    v_state = ThreadState::issued;
                    
                } else {
                    num_threads_sleep = 0;
                    ctrl_state = CtrlState::gather;
                    v_state = ThreadState::sleep;
                    
                    e_state.assign(Config::num_thread, ThreadState::free);
                }
                break;
            
            case ThreadState::issued:
                break;
            
            case ThreadState::sleep:
                throw runtime_error("should not be executed.");
                break;
        }
    }
    
    void CtrlTop::tick_gather() {
        vid_t id_interval_new;
        sid_coalesced tids_assigned;
        
        for (int i = 0; i < Config::num_thread; i++) {
            
            switch (e_state[i]) {
                case ThreadState::free:
                    assert(cnt_shard_processed <= num_shard_total);
                    
                    tids_assigned = std::move(sh->get_new_shards());
                    if (tids_assigned[0] == -1
                        && cnt_shard_processed < num_shard_total) {
                        break;
                    } // shards not ready
                    
                    id_interval_new = sh->get_new_id_interval();
                    if (id_interval_new != v_id_interval) {
                        e_state[i] = ThreadState::sleep;
                        num_threads_sleep++;
                        if (num_threads_sleep == Config::num_thread) {
                            ctrl_state = CtrlState::apply;
                            assert(v_state == ThreadState::sleep);
                            v_state = ThreadState::ready;
                            v_program_cnt = 0;
                        }
                        break;
                    }
                    
                    sh->pop_new_shards();
                    e_id_shard[i] = std::move(tids_assigned);
                    e_state[i] = ThreadState::ready;
                    e_program_cnt[i] = 0;
                    
                    cnt_threads_launched++;
                    break;
                
                case ThreadState::ready:
                    if (e_program_cnt[i] < programs.at(ProgStage::gather).size()) {
                        string instruction = programs.at(ProgStage::gather)[e_program_cnt[i]++];
                        auto callback = [this, i]() {
                            assert(this->e_state[i] == ThreadState::issued);
                            this->e_state[i] = ThreadState::ready;
                        };
                        auto *inst = new Inst(instruction, e_id_shard[i], v_id_interval, i, callback);
                        queue_decode.push_back(inst);
                        e_state[i] = ThreadState::issued;
                        
                    } else {
                        sh->release_shard(e_id_shard[i]);
                        cnt_shard_processed += e_id_shard[i].size();
                        e_id_shard[i].clear();
                        e_state[i] = ThreadState::free;
                        
                    }
                    break;
                
                case ThreadState::issued:
                case ThreadState::sleep:
                    break;
            }
        }
    }
    
    void CtrlTop::tick_apply() {
        switch (v_state) {
            case ThreadState::free:
                throw runtime_error("should not be executed.");
                break;
            
            case ThreadState::ready:
                if (v_program_cnt < programs.at(ProgStage::apply).size()) {
                    string instruction = programs.at(ProgStage::apply)[v_program_cnt++];
                    auto callback = [this]() {
                        assert(this->v_state == ThreadState::issued);
                        this->v_state = ThreadState::ready;
                    };
                    auto *inst = new Inst(instruction, {-1}, v_id_interval,
                                          Config::num_thread, callback);
                    queue_decode.push_back(inst);
                    v_state = ThreadState::issued;
                    
                } else {
                    sh->release_interval(v_id_interval);
                    cnt_interval_processed++;
                    
                    // check whether terminate simulation
                    assert(cnt_interval_processed <= num_interval_total);
                    if (cnt_interval_processed < num_interval_total) {
                        v_state = ThreadState::free;
                        ctrl_state = CtrlState::scatter;
                    } else ctrl_state = CtrlState::end;
                    
                    v_id_interval = -1;
                }
                break;
            
            case ThreadState::issued:
                break;
            
            case ThreadState::sleep:
                throw runtime_error("should not be executed.");
                break;
        }
    }
    
    void CtrlTop::tick() {
        decoder->tick();
        
        switch (ctrl_state) {
            case CtrlState::scatter:
                tick_scatter();
                break;
            case CtrlState::gather:
                tick_gather();
                break;
            case CtrlState::apply:
                tick_apply();
                break;
            case CtrlState::end:
                break;
        }
    }
    
    bool CtrlTop::callback_decode(Op *op) {
#ifdef _DEBUG_EXE
        cout << string("  DECODE end for stream ") + to_string(sid) + "\n";
#endif
        if (op->target == EDevice::VU) {
            queue_issue_vector.push_back(op);
        } else if (op->target == EDevice::MU) {
            queue_issue_matrix.push_back(op);
        } else if (op->target == EDevice::MEM) {
            queue_issue_memory.push_back(op);
        } else throw runtime_error("issue target error.\n");
        return true;
    }
}