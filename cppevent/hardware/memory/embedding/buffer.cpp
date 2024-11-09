//
// Created by SmartCabbage on 2021/7/23.
//

#include "buffer.h"

#ifndef DEBUG_CACHE
#define debug(...)
#else
#define debug(...) do { \
          printf("\033[36m[DEBUG] %s ", __FUNCTION__); \
          printf(__VA_ARGS__); \
          printf("\033[0m\n"); \
      } while (0)
#endif

Buffer::Buffer(int size) {
    latency = Config::LATENCY_EB;
}

bool Buffer::send(RequestBuffer *req) {
    cache_total_access++;
    if (req->type_request == TypeReqOp::WRITE) {
        cache_write_access++;
    } else {
        assert(req->type_request == TypeReqOp::READ);
        cache_read_access++;
    }
    hit_list.emplace_back(
            clk + latency + cache_cycle_access, req);
    cache_cycle_access++;
    return true;
}

void Buffer::tick() {
    ++clk;
    cache_cycle_access = 0;
    while (!hit_list.empty()) {
        if (clk >= hit_list.begin()->first) {
            hit_list.begin()->second->callback(
                    hit_list.begin()->second->callback_arg);
            hit_list.pop_front();
        } else break;
    }
}
