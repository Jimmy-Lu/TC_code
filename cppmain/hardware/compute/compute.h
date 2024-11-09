//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_COMPUTE_H
#define CPPMAIN_COMPUTE_H

#include "../message/message.h"

using namespace std;

class TileHub;
class Buffer;
class BufferGroup;

class ComputeUnit {
public:
    explicit ComputeUnit(EDevice, TileHub *, BufferGroup *);
    ~ComputeUnit() = default;
    void issue_compute(Op *op);
    inline bool check_state_ready();
    inline void tick();
    inline double get_utilization(long long) const;

protected:
    TileHub *th;
    BufferGroup *bg;
    
    EDevice name;
    typedef long num_op_t;
    long num_cycles_operator = 0;
    long num_ops_cycle_total = 0;
    
    enum class State : int {
        ready, compute
    } state = State::ready;
    Op *op_target = nullptr;
    
    inline void compute_end();
    virtual void _issue(num_op_t &ops, num_access_t &accesses) = 0;

private:
    double ops_total = 0;
};

inline bool ComputeUnit::check_state_ready() {
    return state == State::ready;
}

inline double ComputeUnit::get_utilization(long long cycle_total) const {
    return double(ops_total) / cycle_total / num_ops_cycle_total;
}

inline void ComputeUnit::tick() {
    if (state != ComputeUnit::State::compute) return;
    
    bool flag_compute_end = false;
    if (--num_cycles_operator <= 0) flag_compute_end = true;
    if (flag_compute_end) compute_end();
}

inline void ComputeUnit::compute_end() {
#ifdef _DEBUG_EXE
    cout << string("  ") + device_to_string.at(name) + " end computing.\n";
#endif
    assert(state == State::compute);
    op_target->callback();
    delete op_target;
    op_target = nullptr;
    state = State::ready;
}

#endif //CPPMAIN_COMPUTE_H
