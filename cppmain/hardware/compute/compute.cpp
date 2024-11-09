//
// Created by Flowerbeach on 2021/2/4.
//
#include "compute.h"
#include "../memory/embedding/buffer.h"
#include "../tiling/offline/tile_hub.h"

ComputeUnit::ComputeUnit(EDevice name, TileHub *th, BufferGroup *bg)
        : name(name), th(th), bg(bg) {}

void ComputeUnit::issue_compute(Op *op) {
    assert(state == State::ready);
    assert(op_target == nullptr);
    assert(num_cycles_operator <= 0);
    num_cycles_operator = 0;
    state = State::compute;
    op_target = op;
    
    num_op_t num_ops_operator = 0;
    num_access_t num_accesses_operator = {0, 0};
    _issue(num_ops_operator, num_accesses_operator);
    assert(num_ops_operator >= 0);
    assert(num_cycles_operator >= 0);
    assert(get<0>(num_accesses_operator) >= 0);
    assert(get<1>(num_accesses_operator) >= 0);
    
    if (num_cycles_operator == 0) num_cycles_operator = 1;
    bg->add_access(num_accesses_operator);
    ops_total += num_ops_operator;

#ifdef _DEBUG_EXE
    cout << string("  ") + device_to_string.at(name)
            + " begin to compute " + to_string(num_cycles_operator) + " cycles." << endl;
#endif
}
