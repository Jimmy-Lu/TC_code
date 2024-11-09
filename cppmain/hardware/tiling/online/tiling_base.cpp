//
// Created by Flowerbeach on 2021/3/28.
//

#include "../../memory/mem_top.h"
#include "tiling_base.h"

namespace tiling {
    TilingUnit::TilingUnit(
            MemoryAccessHandler *mah, TileHubTU *th) : mah(mah), th(th) {
        assert(Config::name_reorder == EReorder::none);
        
        num_edges = mah->get_num_edges();
        num_vertices = mah->get_num_vertices();
        num_partitions = mah->get_num_partitions();
        
        // init partition
        partition_idx = 0;
        partition_begin = 0;
        partition_end = Config::tile_num_dst;
        if (partition_end > num_vertices) partition_end = num_vertices;
        partition_size = partition_end - partition_begin;
        
        src_begin = 0;
        src_end = num_vertices;
    }
    
    void TilingUnit::tick() {
        // todo the driver move to th?
        if (th->check_new_position_tiling()) {
            if (state == State::end) th->set_position_end();
            else if (state == State::ready) begin(th->get_tile_tiling());
        }
        
        if (state == State::ready) {
            num_cycles_idle++;
        } else if (state == State::scan) {
            num_cycles_busy++;
            scan();
        } else if (state == State::partition) {
            num_cycles_busy++;
            partition();
        } else if (state == State::checkout) {
            num_cycles_busy++;
            checkout();
        } else if (state == State::end) {
        } else throw runtime_error("TilingUnit::tick");
    }
    
    void TilingUnit::begin(Tile_T *t) {
        state = State::scan;
        sram_tile = t;
        
        // reset indicators
        t_num_srcs = 0;
        t_num_edges = 0;
        t_num_edges_of_src = 0;
        
        // allocate tile space
        sram_tile->target_vertex_partition_id = partition_idx;
        sram_tile->target_vertex_begin = partition_begin;
        sram_tile->target_vertex_total = partition_size;
        sram_tile->source_vertex_list.resize(Config::tile_max_src_edge >> 1);
        sram_tile->forward_edge_type.resize(Config::tile_max_src_edge);
        sram_tile->forward_edge_idx.resize(Config::tile_max_src_edge);
        sram_tile->forward_source_idx.resize(Config::tile_max_src_edge);
        sram_tile->forward_target_idx.resize(Config::tile_max_src_edge);
    }
    
    void TilingUnit::partition() {
        // set flag last partition tile
        assert (sram_tile != nullptr);
        sram_tile->output = true;
        
        // update partition
        partition_idx++;
        partition_begin = partition_idx * Config::tile_num_dst;
        partition_end = (partition_idx + 1) * Config::tile_num_dst;
        if (partition_end > num_vertices) partition_end = num_vertices;
        partition_size = partition_end - partition_begin;
        
        src_curr = src_begin;
        state = State::checkout;
    }
    
    void TilingUnit::checkout() {
        convert_from_csr_to_csc(sram_tile);
        sram_tile->tile_id = num_tiles_generated++;
        sram_tile->source_vertex_total = t_num_srcs;
        sram_tile->forward_edge_total = t_num_edges;
        th->receive_tile_from_tu(sram_tile);
        tiles.push_back(sram_tile);
        
        t_num_srcs = 0;
        t_num_edges = 0;
        t_num_edges_of_src = 0;
        sram_tile = nullptr;
        
        // check state end
        if (partition_idx >= num_partitions) {
            state = State::end;
            cout << name + " Busy: " << (num_cycles_busy) << " Cycles." << endl;
            cout << name + " Idle: " << (num_cycles_idle) << " Cycles." << endl;
        } else state = State::ready;
    }
    
}

