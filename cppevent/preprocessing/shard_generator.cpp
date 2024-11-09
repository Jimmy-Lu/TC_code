//
// Created by flowe on 2021/7/6.
//

#include "shard_generator.h"

ShardGenerator::ShardGenerator(GraphLoader *gl, int pid)
        : gl(gl), thread_id(pid) {
    target_dir = gl->path_partition;
    num_edges = gl->num_edges;
    num_vertices = gl->num_vertices;
}

void ShardGenerator::end_partitioning() {
    experimental::filesystem::path target_path = target_dir / (to_string(thread_id) + "." + string("check.shard"));
    ofstream fin(target_path);
    fin << num_shards << '\n';
    // printf(" == %d ==   %ld shards finished.\n", thread_id, num_shards);
}

void ShardGenerator::save_all_shards() {
    auto size = shards.size();
    for (vid_t i = 0; i < size; i++) {
        auto *shard = shards[i];
        experimental::filesystem::path target_path = target_dir / (
                to_string(thread_id) + "." + to_string(shard->shard_id) + string(".shard"));
        shard_saver(shard, Config::shard_format, target_path);
        delete shard;
    }
    shards.clear();
}

void ShardGenerator::save_shards() {
    auto size = shards.size() - 1;
    for (vid_t i = 0; i < size; i++) {
        auto *shard = shards[i];
        experimental::filesystem::path target_path = target_dir / (
                to_string(thread_id) + "." + to_string(shard->shard_id) + string(".shard"));
        shard_saver(shard, Config::shard_format, target_path);
        delete shard;
    }
    shards.erase(shards.begin(), shards.end() - 1);
}
