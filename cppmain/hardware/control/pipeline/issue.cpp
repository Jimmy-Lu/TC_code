//
// Created by Flowerbeach on 2021/7/24.
//

#include "issue.h"
#include "../ctrl_top.h"
#include "../../compute/compute.h"
#include "../../memory/mem_top.h"

/*******************************/
/********* CLASS ISSUE *********/
/*******************************/

control::Issue::Issue(control::CtrlTop *sched, MemoryAccessHandler *mah,
                      vector<ComputeUnit *> vus, vector<ComputeUnit *> mus)
        : sched(sched), mah(mah), vus(std::move(vus)), mus(std::move(mus)) {
    num_free_vus = Config::num_vu;
    num_free_mus = Config::num_mu;
}

bool control::Issue::issue(Op *op, int sid) {
    if (op->target == EDevice::VU) {
        queue_to_vu.push_back(op);
        op->callback = [this, sid]() {
            this->sched->callback_issue(sid);
            this->num_free_vus++;
        };
    } else if (op->target == EDevice::MU) {
        queue_to_mu.push_back(op);
        op->callback = [this, sid]() {
            this->sched->callback_issue(sid);
            this->num_free_mus++;
        };
    } else if (op->target == EDevice::MEM) {
        queue_mem.push_back(op);
        op->callback = [this, sid]() {
            this->sched->callback_issue(sid);
        };
    } else throw runtime_error("issue target error.\n");
    
    return true;
}

void control::Issue::tick_queue_vu() {
    if (queue_to_vu.empty()) return;
    if (num_free_vus == 0) return;
    auto *op = queue_to_vu.front();
    for (auto u: vus) {
        if (u->check_state_ready()) {
            u->issue_compute(op);
            queue_to_vu.pop_front();
            num_free_vus--;
            break;
        }
    }
}

void control::Issue::tick_queue_mu() {
    if (queue_to_mu.empty()) return;
    if (num_free_mus == 0) return;
    auto *op = queue_to_mu.front();
    for (auto u: mus) {
        if (u->check_state_ready()) {
            u->issue_compute(op);
            queue_to_mu.pop_front();
            num_free_mus--;
            break;
        }
    }
}

void control::Issue::tick_queue_mem() {
    if (queue_mem.empty()) return;
    auto *op = queue_mem.front();
    if (mah->check_buffer_i_ready(op->thread_id)) {
        mah->issue_access(op);
        queue_mem.pop_front();
    }
}

