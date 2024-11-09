//
// Created by Flowerbeach on 2021/2/4.
//

#include "compute.h"
#include "../memory/embedding/buffer.h"
#include "../shard/shard_buffer.h"

void ComputeUnit::tick() {
    switch (state) {
        case State::compute:
            if (--num_cycles_operator <= 0) {
                state = State::ready;
#ifdef _DEBUG_EXE
                cout << string("  ") + device_to_string.at(name) + " end computing.\n";
#endif
                op_target->callback();
                delete op_target;
                op_target = nullptr;
            }
            break;
        
        case State::ready:
            op_target = fetch_op_from_ctrl();
            if (op_target != nullptr) {
                state = State::compute;
                
                num_cycles_operator = 0;
                
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
            break;
    }
}
