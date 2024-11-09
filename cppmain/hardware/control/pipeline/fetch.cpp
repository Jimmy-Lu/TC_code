//
// Created by Flowerbeach on 2021/7/24.
//

#include "fetch.h"
#include "../ctrl_top.h"
#include "../../compute/compute.h"

/*******************************/
/********* CLASS FETCH *********/
/*******************************/

control::Fetch::Fetch(control::CtrlTop *sched)
        : PipeBase(sched) { programs = load_program(); }

bool control::Fetch::fetch(Stage stage, int program_cnt, int sid) {
    assert(state == ready);
    auto len_state = programs.at(stage).size();
    if (program_cnt < len_state) {
        instruction = programs.at(stage)[program_cnt++];
        latency = Config::LATENCY_SRAM;
        thread_id = sid;
        state = busy;
        return true;
    } else if (program_cnt == len_state) {
        return false;
    } else throw runtime_error("fetch program counter");
}

void control::Fetch::callback() {
    if (sched->callback_fetch(instruction, thread_id)) {
        instruction = "";
        thread_id = -1;
        state = ready;
    }
}
