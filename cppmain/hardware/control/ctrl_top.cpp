//
// Created by Flowerbeach on 2021/2/12.
//
#include "ctrl_top.h"

#include "../compute/compute.h"
#include "../memory/mem_top.h"
#include "../tiling/online/tiling_base.h"

using namespace std;

namespace control {
    CtrlTop::CtrlTop(MemoryAccessHandler *mah, TileHub *th,
                     const vector<ComputeUnit *> &vus,
                     const vector<ComputeUnit *> &mus,
                     tiling::TilingUnit *tu) : th(th), tu(tu) {
        device_fetch = new control::Fetch(this);
        device_decode = new control::Decode(this, th);
        device_issue = new control::Issue(this, mah, vus, mus);
        
        e_tile_ids.assign(Config::num_thread, tid_coalesced());
        e_partition_id.assign(Config::num_thread, -1);
        e_state.assign(Config::num_thread, State::sleep);
        e_program_cnt.assign(Config::num_thread, 0);
        v_stage = Stage::Scatter;
        v_state = State::sleep;
        v_partition_id = -1;
        v_program_cnt = 0;
        
        for (int i = 0; i < Config::num_thread; i++)
            q_available_thread.push_back(i);
        if (Config::name_arch == EArch::base)
            tile_end = mah->get_num_tiles();
        num_partition_total = mah->get_num_partitions();
        num_partition_simple = ceil(double(num_partition_total)
                                    * (double(mah->get_num_vertices())
                                       / mah->get_num_edges()));
    }
    
    CtrlTop::~CtrlTop() {
        delete device_fetch;
        delete device_decode;
        delete device_issue;
    }
    
    void CtrlTop::thread_vp_end() {
        if (v_stage == Stage::Apply) {
            th->release_partition(v_partition_id);
            v_stage = Stage::Scatter;
            v_partition_id = -1;
            flag_t_begin = false;
        } else {
            v_stage = Stage::Apply;
            flag_t_begin = true;
        }
        v_state = State::sleep;
        v_program_cnt = 0;
    }
    
    void CtrlTop::thread_et_end(int sid) {
        th->release_tiles(e_tile_ids[sid]);
        e_program_cnt[sid] = 0;
        e_partition_id[sid] = -1;
        threads_launched_cnt++;
        tiles_processed_cnt += e_tile_ids[sid].size();
        e_tile_ids[sid].clear();
        if (check_have_tile_to_schedule(tiles_scheduled_cnt)) {
            q_available_thread.push_back(sid);
            e_state[sid] = State::sleep;
        } else {
            e_state[sid] = State::end;
            num_threads_end++;
        }
    }
    
    void CtrlTop::schedule_instruction() {
        // instruction-level scheduling
        if (!device_fetch->check_ready()) return;
        
        if (v_state == State::ready) { // vstream
            if (device_fetch->fetch(v_stage, v_program_cnt, Config::num_thread)) {
                assert(v_partition_id != -1);
                v_state = State::issued;
                v_program_cnt++;
                num_inst_issued++;
            } else thread_vp_end();
            return;
            
        } else { // estream
            // always select the smallest tile to issue
            int sid = -1;
            eid_t smallest = -1;
            for (int i = 0; i < Config::num_thread; i++)
                if (e_state[i] == State::ready && (e_tile_ids[i][0] < smallest || smallest == -1)) {
                    smallest = e_tile_ids[i][0];
                    sid = i;
                }
            if (sid >= 0) {
                Stage target_stage = (v_partition_id < num_partition_simple) ? Stage::Gather : Stage::GatherSimple;
                if (device_fetch->fetch(target_stage,
                                        e_program_cnt[sid], sid)) {
                    assert(e_partition_id[sid] != -1);
                    assert(!e_tile_ids[sid].empty());
                    e_state[sid] = State::issued;
                    e_program_cnt[sid]++;
                    num_inst_issued++;
                } else thread_et_end(sid);
                return;
            }
        }
    }
    
    void CtrlTop::schedule_thread() {
        // stream-level scheduling
        vid_t pid_new_ = th->get_new_tile_pid();
        
        if (pid_new_ != v_partition_id) {
            if (v_state == State::sleep) {
                if (q_available_thread.size() == Config::num_thread
                    || num_threads_end == Config::num_thread) {
                    if (v_stage == Stage::Apply) {
                        if (th->check_last_tile_present())
                            v_state = State::ready;
                    } else if (v_stage == Stage::Scatter) {
                        assert(pid_new_ >= 0);
                        v_partition_id = pid_new_;
                        v_state = State::ready;
                    }
                }
            }
        } else if (!q_available_thread.empty() && flag_t_begin) {
            auto tids_assigned = move(th->assign_tile());
            if (tids_assigned[0] == -1) return;
            // coalescing failed
            
            int target_i = q_available_thread.front();
            assert(e_state[target_i] == State::sleep);
            e_state[target_i] = State::ready;
            e_tile_ids[target_i] = move(tids_assigned);
            e_partition_id[target_i] = v_partition_id;
            
            q_available_thread.pop_front();
            tiles_scheduled_cnt++;
        }
    }
    
    bool CtrlTop::callback_fetch(const string &inst, int sid) {
        if (!device_decode->check_ready()) return false;
        if (sid == Config::num_thread) device_decode->decode(inst, v_partition_id);
        else device_decode->decode(inst, e_tile_ids[sid], e_partition_id[sid], sid);
        return true;
    }
    
    bool CtrlTop::callback_decode(Op *op, int sid) {
#ifdef _DEBUG_EXE
        cout << string("  DECODE end for stream ") + to_string(sid) + "\n";
#endif
        device_issue->issue(op, sid);
        return true;
    }
    
    void CtrlTop::callback_issue(int sid) {
        if (sid == Config::num_thread) {
            assert(v_state == State::issued);
            v_state = State::ready;
        } else {
            assert(e_state[sid] == State::issued);
            e_state[sid] = State::ready;
        }
    }
    
    bool CtrlTop::check_have_tile_to_schedule(eid_t cnt_tiles) {
        if (tile_end == -1) {
            tile_end = tu->get_num_tiles_total();
            if (tile_end < 0) return true;
        }
        return cnt_tiles < tile_end;
    }
    
}