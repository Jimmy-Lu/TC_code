//
// Created by Flowerbeach on 2021/7/24.
//

#include "decode.h"
#include "../ctrl_top.h"
#include "../../tiling/offline/tile_hub.h"

/********************************/
/********* CLASS DECODE *********/
/********************************/

control::Decode::Decode(control::CtrlTop *sched, TileHub *th)
        : PipeBase(sched), th(th) {}

void control::Decode::parse_instruction(
        stringstream &iss, int num_oper, int num_addr,
        const tid_coalesced &tids, vid_t pid, int sid) {
    operation->operands.resize(num_oper);
    operation->addresses.resize(num_addr);
    
    string M, B;
    for (int i = 0; i < num_oper; i++) {
        getline(iss, M, ',');
        operation->operands[i] = parse_number(M, tids, pid);
    }
    for (int i = 0; i < num_oper; i++) {
        getline(iss, B, ',');
        parse_address(B, sid, operation->addresses[i]);
    }
}

void control::Decode::parse_address(const string &str_address, int sid, buffer_addr_t &addr) {
    // throw runtime_error("");// todo parse unified memory address
}

int control::Decode::parse_number(string str_number, tid_coalesced tids, vid_t pid) {
    while (str_number.front() == ' ') str_number.erase(0, 1);
    while (str_number.back() == ' ') str_number.erase(str_number.size() - 1, 1);
    
    int rst = 0;
    for (int ii = 0; ii < tids.size(); ii++) {
        auto tid = tids[ii];
        if (str_number == "NSRC") {
            rst += th->get_num_src(tid);
        } else if (str_number == "NDST") {
            return th->get_num_dst(pid);
        } else if (str_number == "NE") {
            rst += th->get_num_edge(tid);
        } else return stoi(str_number);
    }
    return rst;
}

bool control::Decode::decode(const string &instruction,
                             const tid_coalesced &tids,
                             vid_t pid, int sid) {
    assert (instruction != "END");
    
    string op_name_str;
    stringstream iss(instruction);
    getline(iss, op_name_str, ' ');
    ISA op_name_isa = string_to_ISA.at(op_name_str);
    
    operation = new Op(op_name_isa, tids, pid, sid);
    if (Config::exist(op_name_isa, LIST_OP_MEMORY)) {
        parse_instruction(iss, 0, 1, tids, pid, sid);
        operation->target = EDevice::MEM;
    } else if (Config::exist(op_name_isa, LIST_OP_ALU_1_3)) {
        parse_instruction(iss, 1, 3, tids, pid, sid);
        operation->target = EDevice::VU;
    } else if (Config::exist(op_name_isa, LIST_OP_ALU_2_3)) {
        parse_instruction(iss, 2, 3, tids, pid, sid);
        operation->target = EDevice::VU;
    } else if (Config::exist(op_name_isa, LIST_OP_MM_3_3)) {
        parse_instruction(iss, 3, 3, tids, pid, sid);
        operation->target = EDevice::MU;
    } else if (Config::exist(op_name_isa, LIST_OP_GOP_1_2)) {
        parse_instruction(iss, 1, 2, tids, pid, sid);
        operation->target = EDevice::VU;
    } else if (Config::exist(op_name_isa, LIST_OP_SPECIAL)) {
        parse_instruction(iss, 2, 2, tids, pid, sid);
        operation->target = EDevice::VU;
    } else if (op_name_isa == ISA::NONE) {
        operation->target = EDevice::VU;
    } else throw runtime_error("decode error ");
    
    state = busy;
    latency = 1;
    thread_id = sid;
    return true;
}

void control::Decode::callback() {
    if (sched->callback_decode(operation, thread_id)) {
        operation = nullptr;
        thread_id = -1;
        state = ready;
    }
}
