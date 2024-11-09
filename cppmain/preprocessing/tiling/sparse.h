//
// Created by Flowerbeach on 2021/3/28.
//

#ifndef CPPMAIN_TILING_SPARSE_H
#define CPPMAIN_TILING_SPARSE_H

#include "../tile_generator.h"

class Sparse : public TileGenerator {
public:
    Sparse(GraphLoader *gl, int pid) : TileGenerator(gl, pid) {};
    void generate_tiles() override;

protected:
    virtual void select_rows(vid_t &row_curr, const GraphLoader::partition_t &cols_curr);
    void tile_saver(Tile_T *tile, EFormat tile_format,
                    const experimental::filesystem::path &target_path) override;
};

#endif //CPPMAIN_TILING_SPARSE_H
