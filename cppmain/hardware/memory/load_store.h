//
// Created by Flowerbeach on 2021/7/25.
//

#ifndef CPPMAIN_LOAD_STORE_H
#define CPPMAIN_LOAD_STORE_H

#include <utility>

#include "ramulator/Memory.h"
#include "database/tile_holder.h"
#include "database/csc_csr.h"
#include "database/bitmap.h"

#include "../tiling/offline/tile_hub.h"
#include "../message/message.h"

typedef ramulator::Memory<ramulator::HBM> Memory;
typedef ramulator::Controller<ramulator::HBM> DramCtrl;
typedef ramulator::DRAM<ramulator::HBM> DRAM;

class LoadStoreUnit {
public:
    TileHolder *tile_holder = nullptr;
    CSC_CSR *csc_csr = nullptr;
    Bitmap *bitmap = nullptr;

protected:
    Memory *mem;
    TileHub *th = nullptr;
    
    vid_t num_edges = -1;
    vid_t num_vertices = -1;
    vid_t num_partitions = -1;
    long long addr_tile_next = -1;
    long long addr_vertex_begin = -1;
    long long addr_final_begin = -1;
    // long long addr_csr_ptr_begin = -1;
    long long addr_csr_max_begin = -1;
    long long addr_csr_cnt_begin = -1;
    long long addr_csr_idx_begin = -1;
    long long addr_csc_ptr_begin = -1;
    long long addr_csc_idx_begin = -1;
    long long addr_bitmap_begin = -1;
    const int size_element = 8;
    
    deque<RequestDRAM *> queue_dram_reqs_graph;
    deque<RequestDRAM *> queue_dram_reqs_embed;
    void mem_access_adj(RequestLSU *mreq, callback_args_t callback_arg,
                        long long addr, long size_data);
    void mem_access_embed(RequestLSU *mreq, callback_args_t callback_arg,
                          const vid_t *vids, int len_vid);
    
    inline bool check_invalid_tid(eid_t tid) const { return tid >= tile_holder->num_tile_total; }
    inline Tile_T *get_tile_from_th(eid_t tid) const { return th->get_tile_by_tid(tid); }

public:
    
    void set_th(TileHub *th_l) { th = th_l; }
    inline void tick();
    
    // long read_csr_ptr(RequestLSU *req);
    long read_csr_idx(RequestLSU *req);
    long read_csr_cnt(RequestLSU *req);
    long write_csr_cnt(RequestLSU *req);
    long read_csr_max(RequestLSU *req);
    long write_csr_max(RequestLSU *req);
    long read_csc_ptr(RequestLSU *req);
    long read_csc_idx(RequestLSU *req);
    long read_bitmap(RequestLSU *req);
    long read_adj(RequestLSU *req);
    long read_src(RequestLSU *req);
    long read_dst(RequestLSU *req);
    long write_dst(RequestLSU *req);
    
    inline eid_t get_num_tiles() const { return tile_holder->num_tile_total; }
    inline vid_t get_num_vertices() const { return num_vertices; }
    inline vid_t get_num_partitions() const { return num_partitions; }
    inline eid_t get_num_edges() const { return num_edges; }
    
    explicit LoadStoreUnit(Memory *mem) : mem(mem) {
        // todo address mapping
        if (Config::name_arch == EArch::base) {
            tile_holder = new TileHolder();
            num_edges = tile_holder->num_edges;
            num_vertices = tile_holder->num_vertices;
            num_partitions = tile_holder->num_partitions;
            addr_vertex_begin = tile_holder->addr_vertex_begin;
            addr_final_begin = tile_holder->addr_final_begin;
            addr_tile_next = 0;
        } else if (Config::name_arch == EArch::tu) {
            if (Config::name_tu == ETU::csc
                || Config::name_tu == ETU::csr) {
                csc_csr = new CSC_CSR();
                num_edges = csc_csr->num_edges;
                num_vertices = csc_csr->num_vertices;
                num_partitions = csc_csr->num_partitions;
                
                addr_vertex_begin = csc_csr->addr_vertex_begin;
                addr_final_begin = csc_csr->addr_final_begin;
                addr_csr_max_begin = 0;
                addr_csr_cnt_begin = (num_vertices + 1) * size_element + addr_csr_max_begin;
                addr_csr_idx_begin = (num_vertices + 1) * size_element * 2 + addr_csr_cnt_begin;
                addr_csc_ptr_begin = (num_edges) * size_element + addr_csr_idx_begin;
                addr_csc_idx_begin = (num_vertices + 1) * size_element + addr_csc_ptr_begin;
            } else if (Config::name_tu == ETU::bitmap) {
                bitmap = new Bitmap();
                num_edges = bitmap->num_edges;
                num_vertices = bitmap->num_vertices;
                num_partitions = bitmap->num_partitions;
                addr_vertex_begin = bitmap->addr_vertex_begin;
                addr_final_begin = bitmap->addr_final_begin;
                addr_bitmap_begin = bitmap->addr_bitmap_begin;
            } else throw runtime_error(tu_to_string.at(Config::name_tu));
        } else throw runtime_error(arch_to_string.at(Config::name_arch));
    }
    
    ~LoadStoreUnit() {
        delete tile_holder;
        delete csc_csr;
        delete bitmap;
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
