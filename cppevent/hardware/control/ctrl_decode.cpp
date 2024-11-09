//
// Created by Flowerbeach on 2021/7/24.
//

#include "ctrl_decode.h"
#include "ctrl_top.h"
#include "../shard/shard_buffer.h"

/********************************/
/********* CLASS DECODE *********/
/********************************/

control::Decode::Decode(control::CtrlTop *sched, ShardHub *th)
        : ctrl(sched), sh(th) {}

void control::Decode::parse_instruction(
        stringstream &iss, int num_oper, int num_addr,
        const sid_coalesced &sids, id_interval_t iid, id_thread_t tid) {
    operation->operands.resize(num_oper);
    operation->addresses.resize(num_addr);
    
    string M, B;
    for (int i = 0; i < num_oper; i++) {
        getline(iss, M, ',');
        operation->operands[i] = parse_number(M, sids, iid);
    }
    for (int i = 0; i < num_oper; i++) {
        getline(iss, B, ',');
        parse_address(B, tid, operation->addresses[i]);
    }
}

void control::Decode::parse_address(const string &str_address,
                                    id_thread_t tid, buffer_addr_t &addr) {
    // throw runtime_error("");// todo parse unified memory address
}

int control::Decode::parse_number(string str_number, const sid_coalesced &sids, id_interval_t iid) {
    while (str_number.front() == ' ') str_number.erase(0, 1);
    while (str_number.back() == ' ') str_number.erase(str_number.size() - 1, 1);
    
    int rst = 0;
    for (int sid: sids) {
        if (str_number == "NSRC") {
            rst += sh->get_num_src(sid);
        } else if (str_number == "NDST") {
            return sh->get_num_dst(iid);
        } else if (str_number == "NE") {
            rst += sh->get_num_edge(sid);
        } else return stoi(str_number);
    }
    return rst;
}

bool control::Decode::decode(const control::Inst *inst) {
    assert(inst->instruction != "END");
    
    string op_name_str;
    stringstream iss(inst->instruction);
    getline(iss, op_name_str, ' ');
    ISA op_name_isa = string_to_ISA.at(op_name_str);
    
    operation = new Op(op_name_isa, inst->shards_id, inst->interval_id, inst->thread_id, inst->callback);
    if (Config::exist(op_name_isa, LIST_OP_MEMORY)) {
        parse_instruction(iss, 0, 1, inst->shards_id, inst->interval_id, inst->thread_id);
        operation->target = EDevice::MEM;
    } else if (Config::exist(op_name_isa, LIST_OP_ALU_1_3)) {
        parse_instruction(iss, 1, 3, inst->shards_id, inst->interval_id, inst->thread_id);
        operation->target = EDevice::VU;
    } else if (Config::exist(op_name_isa, LIST_OP_ALU_2_3)) {
        parse_instruction(iss, 2, 3, inst->shards_id, inst->interval_id, inst->thread_id);
        operation->target = EDevice::VU;
    } else if (Config::exist(op_name_isa, LIST_OP_MM_3_3)) {
        parse_instruction(iss, 3, 3, inst->shards_id, inst->interval_id, inst->thread_id);
        operation->target = EDevice::MU;
    } else if (Config::exist(op_name_isa, LIST_OP_GOP_1_2)) {
        parse_instruction(iss, 1, 2, inst->shards_id, inst->interval_id, inst->thread_id);
        operation->target = EDevice::VU;
    } else if (Config::exist(op_name_isa, LIST_OP_SPECIAL)) {
        parse_instruction(iss, 2, 2, inst->shards_id, inst->interval_id, inst->thread_id);
        operation->target = EDevice::VU;
    } else if (op_name_isa == ISA::NONE) {
        operation->target = EDevice::VU;
    } else throw runtime_error("decode error ");
    
    state = busy;
    latency = 1 + Config::LATENCY_SRAM;
    return true;
}

void control::Decode::tick() {
    switch (state) {
        case DecodeState::busy:
            if (--latency <= 0) {
                ctrl->callback_decode(operation);
                operation = nullptr;
                state = ready;
            }
            break;
        case DecodeState::ready:
            if (!ctrl->queue_decode.empty()) {
                auto *inst = ctrl->queue_decode.front();
                ctrl->queue_decode.pop_front();
                decode(inst);
                state = busy;
            }
            break;
    }
}