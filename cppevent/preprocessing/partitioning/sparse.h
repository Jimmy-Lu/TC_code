//
// Created by Flowerbeach on 2021/3/28.
//

#ifndef CPPMAIN_PARTITIONING_SPARSE_H
#define CPPMAIN_PARTITIONING_SPARSE_H

#include "../shard_generator.h"

class Sparse : public ShardGenerator {
public:
    Sparse(GraphLoader *gl, int pid) : ShardGenerator(gl, pid) {};
    void generate_shards() override;

protected:
    virtual void select_rows(vid_t &row_curr, const GraphLoader::interval_t &cols_curr);
    void shard_saver(Shard_T *shard, EFormat shard_format,
                     const experimental::filesystem::path &target_path) override;
};

#endif //CPPMAIN_PARTITIONING_SPARSE_H
