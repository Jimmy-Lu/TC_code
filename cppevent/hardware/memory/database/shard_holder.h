//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_SHARD_HOLDER_H
#define CPPMAIN_SHARD_HOLDER_H

#include <experimental/filesystem>
#include "../../../configs/shard.h"

class ShardHolder {
public:
    explicit ShardHolder();
    
    // long long addr_edge_begin;
    long long addr_vertex_begin;
    long long addr_final_begin;
    long long addr_shard_begin;
    
    eid_t num_edges = -1;
    vid_t num_vertices = -1;
    id_shard_t num_shard_total = 0;
    id_interval_t num_intervals = -1;
    
    Shard_T *get_shard_i(eid_t tid) const;

protected:
    experimental::filesystem::path target_dir;
    vector<vid_t> degree_vertex;
    vector<Shard_T *> shards;
    
    void load_meta();
    void load_shards();
};

#endif //CPPMAIN_SHARD_HOLDER_H
