//
// Created by Flowerbeach on 2021/2/6.
//

#include "shard_buffer.h"
#include "../memory/mem_top.h"

ShardHub::ShardHub(MemoryAccessHandler *mah) : mah(mah) {
    num_shard_onchip = Config::num_thread * max_shard_coalesced;
    
    p_data.assign(num_shard_onchip, nullptr);
    p_state.assign(num_shard_onchip, State::READY);
    for (int i = 0; i < num_shard_onchip; i++)
        queue_pi_empty.push_back(i);
    shard_to_assign.resize(1);
}

void ShardHub::issue_to_mah(int i) {
    assert(p_state[i] == State::READY);
    p_state[i] = State::LOADING;
    auto *req = new RequestLSU();
    req->type_data = TypeData::adj;
    req->type_req = TypeReqOp::READ;
    req->cb_args = new callback_args_t({{"shard_id", shard_id_to_load}});
    req->source_device = "ShardHub" + to_string(i);
    req->callback = get_function_callback(i);
    mah->issue_access(req);
    shard_id_to_load += 1;
}