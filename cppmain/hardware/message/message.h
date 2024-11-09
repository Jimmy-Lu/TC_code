//
// Created by Flowerbeach on 2021/7/24.
//

#ifndef CPPMAIN_ARCH_MESSAGE_H
#define CPPMAIN_ARCH_MESSAGE_H

#include <utility>
#include "../../configs/tile.h"
#include "../../configs/bitmap.h"
#include "../../configs/config.h"
#include "../../algorithm/instruction_set.h"
#include "../memory/ramulator/Request.h"

typedef tuple<long, long> num_access_t; //read, write


const int max_tile_coalesced = 64;
typedef vector<eid_t> tid_coalesced;
typedef vector<Tile_T *> tile_coalesced;
// const int threshold_min_src = 32 * 2;

class buffer_addr_t {
    int offset = -1;
    int bid = -1;
};

class Op {
public:
    ISA name;
    EDevice target;
    vector<int> operands;
    vector<buffer_addr_t> addresses;
    tid_coalesced tile_ids;
    vid_t partition_id;
    int thread_id;
    
    Op(ISA name, tid_coalesced tids, vid_t pid, int sid)
            : name(name), tile_ids(std::move(tids)), partition_id(pid), thread_id(sid) {}
    
    function<void()> callback = nullptr;
};

enum class TypeData {
    adj, src, dst, weight,
    csc_ptr, csc_idx, csr_ptr, csr_idx, csr_cnt_r, csr_cnt_w,
    csr_max_r, csr_max_w,
    bitmap,
};

typedef ramulator::Request::Type TypeReqOp;
typedef ramulator::Request RequestDRAM;

class RequestLSU {
public:
    RequestLSU() = default;
    ~RequestLSU() { delete cb_args; }
    
    int thread_id = -2;
    TypeReqOp type_req;
    TypeData type_data;
    string source_device;
    
    vector<vid_t> *sram;
    BitmapSingle **bitmap;
    tid_coalesced tile_ids;
    
    callback_args_t *cb_args;
    callback_t callback;
};

class RequestBuffer {
public:
    TypeReqOp type_request;
    
    long addr = -1;
    string name_buffer;
    int thread_id;
    
    callback_args_t callback_arg;
    callback_t callback = nullptr; // callback to buffer
};

#endif //CPPMAIN_ARCH_MESSAGE_H
