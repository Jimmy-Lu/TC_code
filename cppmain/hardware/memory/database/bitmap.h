//
// Created by Flowerbeach on 2022/2/24.
//

#include <cassert>
#include <cstring>
#include <utility>

#include "../../../configs/bitmap.h"

#ifndef CPPMAIN_DATABASE_BITMAP_H
#define CPPMAIN_DATABASE_BITMAP_H

class Bitmap {
public:
    BitmapMultiple *bitmap;
    
    eid_t num_edges = -1;
    vid_t num_vertices = -1;
    vid_t num_partitions = -1;
    long long addr_vertex_begin;
    long long addr_final_begin;
    long long addr_bitmap_begin;
    
    Bitmap() {
        bitmap = new BitmapMultiple();
        num_edges = bitmap->num_edges;
        num_vertices = bitmap->num_vertices;
        
        addr_vertex_begin = 0;
        addr_final_begin = num_vertices * Config::SIZE_FEATURE_BYTE + addr_vertex_begin;
        addr_bitmap_begin = num_vertices * Config::SIZE_FEATURE_BYTE + addr_final_begin;
        num_partitions = ceil(double(num_vertices) / Config::tile_num_dst);
        
        printf("---- Graph Property ----\n");
        printf("  # Vertex:  %d\n", num_vertices);
        printf("  #  Edge:   %d\n", num_edges);
        printf("------------------------\n");
    }
    ~Bitmap() { delete bitmap; }
    
};

#endif //CPPMAIN_DATABASE_BITMAP_H
