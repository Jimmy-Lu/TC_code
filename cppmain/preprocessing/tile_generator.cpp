//
// Created by flowe on 2021/7/6.
//

#include "tile_generator.h"

TileGenerator::TileGenerator(GraphLoader *gl, int pid)
        : gl(gl), thread_id(pid) {
    target_dir = gl->path_partition;
    num_edges = gl->num_edges;
    num_vertices = gl->num_vertices;
}

void TileGenerator::end_partition() {
    experimental::filesystem::path target_path = target_dir / (to_string(thread_id) + "." + string("check.tile"));
    ofstream fin(target_path);
    fin << num_tiles << '\n';
    // printf(" == %d ==   %ld tiles finished.\n", thread_id, num_tiles);
}

void TileGenerator::save_all_tiles() {
    auto size = tiles.size();
    for (vid_t i = 0; i < size; i++) {
        auto *tile = tiles[i];
        experimental::filesystem::path target_path = target_dir / (
                to_string(thread_id) + "." + to_string(tile->tile_id) + string(".tile"));
        tile_saver(tile, Config::tile_format, target_path);
        delete tile;
    }
    tiles.clear();
}

void TileGenerator::save_tiles() {
    auto size = tiles.size() - 1;
    for (vid_t i = 0; i < size; i++) {
        auto *tile = tiles[i];
        experimental::filesystem::path target_path = target_dir / (
                to_string(thread_id) + "." + to_string(tile->tile_id) + string(".tile"));
        tile_saver(tile, Config::tile_format, target_path);
        delete tile;
    }
    tiles.erase(tiles.begin(), tiles.end() - 1);
}
