//
// Created by flowe on 2021/7/6.
//

#ifndef CPPMAIN_SHARD_GENERATOR_H
#define CPPMAIN_SHARD_GENERATOR_H

#include "graph_loader.h"

class ShardGenerator {
public:
    const int max_shard = 8;
    ShardGenerator(GraphLoader *, int pid);
    virtual ~ShardGenerator() = default;
    virtual void generate_shards() = 0;
    void end_partitioning();
    
    vid_t get_empty_rows() const { return empty_rows; }
    
    vector<double> vec_util;

protected:
    int thread_id;
    GraphLoader *gl;
    
    fs::path target_dir;
    int num_edges;
    int num_vertices;
    vid_t empty_rows = 0;
    eid_t num_shards = 0;
    deque<Shard_T *> shards;
    void save_all_shards();
    void save_shards();

protected:
    virtual void shard_saver(Shard_T *shard, EFormat shard_format,
                             const experimental::filesystem::path &target_path) = 0;
};

#endif //CPPMAIN_SHARD_GENERATOR_H
