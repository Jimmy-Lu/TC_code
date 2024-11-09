//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_TILE_HOLDER_H
#define CPPMAIN_TILE_HOLDER_H

#include <experimental/filesystem>
#include "../../../configs/tile.h"

class TileHolder {
public:
    explicit TileHolder();
    
    // long long addr_edge_begin;
    long long addr_vertex_begin;
    long long addr_final_begin;
    long long addr_tile_begin;
    
    eid_t num_edges = -1;
    vid_t num_vertices = -1;
    vid_t num_partitions = -1;
    eid_t num_tile_total = 0;
    
    Tile_T *get_tile_i(eid_t tid) const;

protected:
    experimental::filesystem::path target_dir;
    vector<vid_t> degree_vertex;
    vector<Tile_T *> tiles;
    
    void load_meta();
    void load_tiles();
};

#endif //CPPMAIN_TILE_HOLDER_H
