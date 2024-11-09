//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_COMPUTE_H
#define CPPMAIN_COMPUTE_H

#include "../message/message.h"

using namespace std;

class ShardHub;
class Buffer;
class BufferGroup;

namespace control {
    class CtrlTop;
}

class ComputeUnit {
public:
    explicit ComputeUnit(EDevice name, ShardHub *sh, BufferGroup *bg)
            : name(name), sh(sh), bg(bg) {}
    ~ComputeUnit() = default;
    inline bool check_state_ready();
    void set_ctrl(control::CtrlTop *c) { this->ctrl = c; }
    void tick();
    
    inline double get_utilization(long long) const;

protected:
    ShardHub *sh = nullptr;
    BufferGroup *bg = nullptr;
    control::CtrlTop *ctrl = nullptr;
    
    EDevice name;
    typedef long num_op_t;
    long num_cycles_operator = 0;
    long num_ops_cycle_total = 0;
    
    enum class State : int {
        ready, compute
    } state = State::ready;
    Op *op_target = nullptr;
    
    virtual void _issue(num_op_t &ops, num_access_t &accesses) = 0;
    virtual Op *fetch_op_from_ctrl() = 0;

private:
    double ops_total = 0;
};

inline bool ComputeUnit::check_state_ready() {
    return state == State::ready;
}

inline double ComputeUnit::get_utilization(long long cycle_total) const {
    return double(ops_total) / cycle_total / num_ops_cycle_total;
}

#endif //CPPMAIN_COMPUTE_H
