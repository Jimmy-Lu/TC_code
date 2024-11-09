//
// Created by flowe on 2021/7/6.
//

#ifndef CPPMAIN_TILE_GENERATOR_H
#define CPPMAIN_TILE_GENERATOR_H

#include "graph_loader.h"

class TileGenerator {
public:
    const int max_tile = 8;
    TileGenerator(GraphLoader *, int pid);
    virtual ~TileGenerator() = default;
    virtual void generate_tiles() = 0;
    void end_partition();
    
    vid_t get_empty_rows() const { return empty_rows; }
    
    vector<double> vec_util;

protected:
    int thread_id;
    GraphLoader *gl;
    
    fs::path target_dir;
    int num_edges;
    int num_vertices;
    vid_t empty_rows = 0;
    eid_t num_tiles = 0;
    deque<Tile_T *> tiles;
    void save_all_tiles();
    void save_tiles();

protected:
    virtual void tile_saver(Tile_T *tile, EFormat tile_format,
                            const experimental::filesystem::path &target_path) = 0;
};

#endif //CPPMAIN_TILE_GENERATOR_H
