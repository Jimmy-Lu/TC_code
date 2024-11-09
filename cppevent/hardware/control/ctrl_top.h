//
// Created by SmartCabbage on 2021/1/27.
//

#ifndef CPPMAIN_CTRL_TOP_H
#define CPPMAIN_CTRL_TOP_H

#include "../message/message.h"
#include "ctrl_decode.h"

class ShardHub;
class ComputeUnit;
class MemoryAccessHandler;

namespace control {
    class Decode;
    
    class CtrlTop {
    public:
        CtrlTop(MemoryAccessHandler *mah, ShardHub *th,
                const vector<ComputeUnit *> &vus,
                const vector<ComputeUnit *> &mus);
        ~CtrlTop();
        
        void tick();
        void tick_scatter();
        void tick_gather();
        void tick_apply();
        
        bool callback_decode(Op *op);
        
        enum class CtrlState {
            scatter, gather, apply, end
        };
        CtrlState ctrl_state = CtrlState::scatter;
    
    protected:
        ShardHub *sh = nullptr;
        
        enum class ThreadState {
            free, ready, issued, sleep
        };
        
        map<ProgStage, vector<string>> programs = load_program();
        
        // for vertex partition
        ThreadState v_state;
        id_interval_t v_id_interval;
        int v_program_cnt;
        
        // for edge shard
        vector<ThreadState> e_state;
        vector<sid_coalesced> e_id_shard;
        vector<int> e_program_cnt;
        
        int num_threads_sleep = 0;
    
    public:
        typedef deque<Inst *> QueueDecode;
        QueueDecode queue_decode;
        
        typedef deque<Op *> QueueIssue;
        QueueIssue queue_issue_memory;
        QueueIssue queue_issue_matrix;
        QueueIssue queue_issue_vector;
        
        id_shard_t num_shard_total;
        id_shard_t cnt_shard_processed = 0;
        id_shard_t cnt_threads_launched = 0;
        id_interval_t num_interval_total;
        id_interval_t cnt_interval_processed = 0;
    
    private:
        Decode *decoder;
        
    };
    
}
#endif //CPPMAIN_CTRL_TOP_H
