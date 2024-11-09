//
// Created by Flowerbeach on 2021/2/6.
//

#include "tile_hub.h"
#include "../../memory/mem_top.h"

TileHub::TileHub(MemoryAccessHandler *mah) : mah(mah) {
    if (Config::name_arch == EArch::tu) {
        num_tile_onchip = Config::num_tu + 1;
        throw runtime_error("what does num_tu refer to?");
    } else if (Config::name_arch == EArch::base) {
        num_tile_onchip = Config::num_thread * max_tile_coalesced;
        // num_tile_onchip = Config::num_thread + 1;
    } else throw runtime_error("TileHub::TileHub");
    
    p_data.assign(num_tile_onchip, nullptr);
    p_state.assign(num_tile_onchip, State::READY);
    for (int i = 0; i < num_tile_onchip; i++)
        q_pi_empty.push_back(i);
    tile_to_assign.resize(1);
}

void TileHub::issue_to_mah(int i) {
    assert(p_state[i] == State::READY);
    p_state[i] = State::LOADING;
    auto *req = new RequestLSU();
    req->type_data = TypeData::adj;
    req->type_req = TypeReqOp::READ;
    req->cb_args = new callback_args_t({{"tile_id", tile_to_load_id}});
    req->source_device = "TileHub" + to_string(i);
    req->callback = get_function_callback(i);
    mah->issue_access(req);
    tile_to_load_id += 1;
}