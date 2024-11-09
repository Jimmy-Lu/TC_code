//
// Created by Flowerbeach on 2021/7/24.
//

#ifndef CPPMAIN_CTRL_STAGES_H
#define CPPMAIN_CTRL_STAGES_H

#include <cassert>
#include <functional>

class ComputeUnit;

namespace control {
    class CtrlTop;
    
    class PipeBase {
    protected:
        CtrlTop *sched = nullptr;
        enum State {
            busy, ready
        } state = ready;
        int thread_id = -1;
        int latency = 0;
    
    public:
        explicit PipeBase(CtrlTop *sched) : sched(sched) {};
        virtual void callback() = 0;
        inline bool check_ready();
        inline void tick();
    };
}

inline void control::PipeBase::tick() {
    if (state == ready) return;
    if (--latency <= 0) {
        callback();
        latency = 0;
    }
}

inline bool control::PipeBase::check_ready() {
    return state == ready;
}

#endif //CPPMAIN_CTRL_STAGES_H
