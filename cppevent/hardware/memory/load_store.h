//
// Created by Flowerbeach on 2021/7/25.
//

#ifndef CPPMAIN_LOAD_STORE_H
#define CPPMAIN_LOAD_STORE_H

#include <utility>

#include "ramulator/Memory.h"
#include "database/shard_holder.h"

#include "../shard/shard_buffer.h"
#include "../message/message.h"

typedef ramulator::Memory<ramulator::HBM> Memory;
typedef ramulator::Controller<ramulator::HBM> DramCtrl;
typedef ramulator::DRAM<ramulator::HBM> DRAM;

class LoadStoreUnit {
public:
    ShardHolder *shard_holder = nullptr;

protected:
    Memory *mem;
    ShardHub *sh = nullptr;
    
    vid_t num_edges = -1;
    vid_t num_vertices = -1;
    id_interval_t num_intervals = -1;
    long long addr_shard_next = -1;
    long long addr_vertex_begin = -1;
    long long addr_final_begin = -1;
    
    deque<RequestDRAM *> queue_dram_reqs_graph;
    deque<RequestDRAM *> queue_dram_reqs_embed;
    void mem_access_adj(RequestLSU *mreq, callback_args_t callback_arg,
                        long long addr, long size_data);
    void mem_access_embed(RequestLSU *mreq, callback_args_t callback_arg,
                          const vid_t *vids, int len_vid);
    
    inline bool check_invalid_sid(id_shard_t sid) const { return sid >= shard_holder->num_shard_total; }
    inline Shard_T *get_shard_from_sh(id_shard_t sid) const { return sh->get_shard_by_id(sid); }

public:
    
    void set_sh(ShardHub *sh_l) { sh = sh_l; }
    inline void tick();
    
    // long read_csr_ptr(RequestLSU *req);
    long read_adj(RequestLSU *req);
    long read_src(RequestLSU *req);
    long read_dst(RequestLSU *req);
    long write_dst(RequestLSU *req);
    
    inline eid_t get_num_edges() const { return num_edges; }
    inline vid_t get_num_vertices() const { return num_vertices; }
    inline id_shard_t get_num_shards() const { return shard_holder->num_shard_total; }
    inline id_interval_t get_num_intervals() const { return num_intervals; }
    
    explicit LoadStoreUnit(Memory *mem) : mem(mem) {
        // todo address mapping
        shard_holder = new ShardHolder();
        num_edges = shard_holder->num_edges;
        num_vertices = shard_holder->num_vertices;
        num_intervals = shard_holder->num_intervals;
        addr_vertex_begin = shard_holder->addr_vertex_begin;
        addr_final_begin = shard_holder->addr_final_begin;
        addr_shard_next = 0;
    }
    
    ~LoadStoreUnit() {
        delete shard_holder;
    }
    
};

const int num_issues = 8;
inline void LoadStoreUnit::tick() {
    // use only one dram module
    for (int i = 0; i < num_issues; i++) {
        if (!queue_dram_reqs_graph.empty()) {
            auto *req = queue_dram_reqs_graph.front();
            if (mem->send(*req)) {
                queue_dram_reqs_graph.pop_front();
                delete req;
            } else break;
        } else if (!queue_dram_reqs_embed.empty()) {
            auto *req = queue_dram_reqs_embed.front();
            if (mem->send(*req)) {
                queue_dram_reqs_embed.pop_front();
                delete req;
            } else break;
        }
    }
    // // use two dram modules for graph and embedding
    // if (!queue_dram_reqs_graph.empty()) {
    //     auto *req = queue_dram_reqs_graph.front();
    //     if (mems[int(req->is_embedding)]->send(*req)) {
    //         queue_dram_reqs_graph.pop_front();
    //         delete req;
    //     }
    // }
    // if (!queue_dram_reqs_embed.empty()) {
    //     auto *req = queue_dram_reqs_embed.front();
    //     if (mems[int(req->is_embedding)]->send(*req)) {
    //         queue_dram_reqs_embed.pop_front();
    //         delete req;
    //     }
    // }
}

#endif //CPPMAIN_LOAD_STORE_H
