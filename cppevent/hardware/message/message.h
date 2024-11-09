//
// Created by Flowerbeach on 2021/7/24.
//

#ifndef CPPMAIN_ARCH_MESSAGE_H
#define CPPMAIN_ARCH_MESSAGE_H

#include <utility>
#include "../../configs/shard.h"
#include "../../configs/config.h"
#include "../../algorithm/instruction_set.h"
#include "../memory/ramulator/Request.h"

typedef tuple<long, long> num_access_t; //read, write


const int max_shard_coalesced = 8;
typedef vector<id_shard_t> sid_coalesced;
typedef vector<Shard_T *> shard_coalesced;
// const int threshold_min_src = 32 * 2;

class buffer_addr_t {
    int offset = -1;
    int bid = -1;
};

class Op {
public:
    ISA name;
    EDevice target = EDevice::none;
    vector<int> operands;
    vector<buffer_addr_t> addresses;
    sid_coalesced shard_ids;
    id_interval_t interval_id;
    id_thread_t thread_id;
    
    function<void()> callback;
    
    Op(ISA name, sid_coalesced sids, id_interval_t iid,
       id_thread_t tid, function<void()> callback)
            : name(name), shard_ids(std::move(sids)),
              interval_id(iid), thread_id(tid),
              callback(std::move(callback)) {}
    
};

enum class TypeData {
    adj, src, dst, weight,
};

typedef ramulator::Request::Type TypeReqOp;
typedef ramulator::Request RequestDRAM;

class RequestLSU {
public:
    RequestLSU() = default;
    ~RequestLSU() { delete cb_args; }
    
    id_thread_t thread_id = -2;
    TypeReqOp type_req;
    TypeData type_data;
    string source_device;
    
    vector<vid_t> *sram;
    sid_coalesced shard_ids;
    
    callback_args_t *cb_args;
    callback_t callback;
};

class RequestBuffer {
public:
    TypeReqOp type_request;
    
    long addr = -1;
    string name_buffer;
    id_thread_t thread_id;
    
    callback_args_t callback_arg;
    callback_t callback = nullptr; // callback to buffer
};

#endif //CPPMAIN_ARCH_MESSAGE_H
