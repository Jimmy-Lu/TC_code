//
// Created by Flowerbeach on 2021/7/24.
//

#ifndef CPPMAIN_CTRL_ISSUE_H
#define CPPMAIN_CTRL_ISSUE_H

#include "../../message/message.h"

class ComputeUnit;
class MemoryAccessHandler;

namespace control {
    class CtrlTop;
    
    class Issue {
    protected:
        CtrlTop *sched;
        MemoryAccessHandler *mah;
        deque<Op *> queue_mem;
        deque<Op *> queue_to_vu;
        deque<Op *> queue_to_mu;
        vector<ComputeUnit *> vus;
        vector<ComputeUnit *> mus;
        int num_free_vus;
        int num_free_mus;
    
    public:
        explicit Issue(control::CtrlTop *sched, MemoryAccessHandler *mah,
                       vector<ComputeUnit *> vus, vector<ComputeUnit *> mus);
        bool issue(Op *op, int stream_id);
        void tick_queue_vu();
        void tick_queue_mu();
        void tick_queue_mem();
        inline void tick();
        
    };
}

inline void control::Issue::tick() {
    tick_queue_vu();
    tick_queue_mu();
    tick_queue_mem();
}

#endif //CPPMAIN_CTRL_ISSUE_H
