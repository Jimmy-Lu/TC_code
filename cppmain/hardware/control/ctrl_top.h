//
// Created by SmartCabbage on 2021/1/27.
//

#ifndef CPPMAIN_CTRL_TOP_H
#define CPPMAIN_CTRL_TOP_H

#include "../message/message.h"
#include "pipeline/fetch.h"
#include "pipeline/decode.h"
#include "pipeline/issue.h"

namespace tiling {
    class TilingUnit;
}

class TileHub;
class ComputeUnit;
class MemoryAccessHandler;

namespace control {
    class Fetch;
    class Decode;
    class Issue;
    
    class CtrlTop {
    public:
        CtrlTop(MemoryAccessHandler *mah, TileHub *th,
                const vector<ComputeUnit *> &vus,
                const vector<ComputeUnit *> &mus,
                tiling::TilingUnit *tu = nullptr);
        ~CtrlTop();
        
        inline void tick();
        void thread_vp_end();
        void thread_et_end(int sid);
        
        bool callback_fetch(const string &inst, int sid);
        bool callback_decode(Op *op, int sid);
        void callback_issue(int sid);
        
        inline eid_t get_tiles_processed() const { return tiles_processed_cnt; }
        inline eid_t get_threads_launched() const { return threads_launched_cnt; }
    
    protected:
        TileHub *th = nullptr;
        tiling::TilingUnit *tu = nullptr;
        
        enum class State {
            ready, issued, sleep, end
        };
        
        // for vertex partition
        State v_state;
        Stage v_stage;
        vid_t v_partition_id;
        int v_program_cnt;
        
        // for edge tile
        vector<State> e_state;
        vector<tid_coalesced> e_tile_ids;
        vector<vid_t> e_partition_id;
        vector<int> e_program_cnt;
        deque<int> q_available_thread;
        int num_threads_end = 0;
        
        eid_t tile_end = -1;
        eid_t tiles_processed_cnt = 0;
        eid_t tiles_scheduled_cnt = 0;
        eid_t threads_launched_cnt = 0;
        long long num_inst_issued = 0;
        
        vid_t num_partition_total;
        vid_t num_partition_simple;
    
    private:
        Fetch *device_fetch;
        Decode *device_decode;
        Issue *device_issue;
        
        void schedule_thread();
        void schedule_instruction();
        bool flag_t_begin = false;
        
        bool check_have_tile_to_schedule(eid_t cnt_tiles);
        
    };
    
    inline void CtrlTop::tick() {
        // Timer timer0;
        // timer0.Start();
        
        device_issue->tick();
        
        // timer0.Stop();
        // timer0.PrintMicroSecond("device_issue->tick(): ");
        // Timer timer1;
        // timer1.Start();
        
        device_decode->tick();
        
        // timer1.Stop();
        // timer1.PrintMicroSecond("device_decode->tick(): ");
        // Timer timer2;
        // timer2.Start();
        
        device_fetch->tick();
        
        // timer2.Stop();
        // timer2.PrintMicroSecond("device_fetch->tick(): ");
        // Timer timer3;
        // timer3.Start();
        
        schedule_instruction();
        
        // timer3.Stop();
        // timer3.PrintMicroSecond("schedule_instruction: ");
        // Timer timer4;
        // timer4.Start();
        
        schedule_thread();
        
        // timer4.Stop();
        // timer4.PrintMicroSecond("schedule_thread: ");
        
    }
}
#endif //CPPMAIN_CTRL_TOP_H
