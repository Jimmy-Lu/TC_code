//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_TILE_H
#define CPPMAIN_TILE_H

#include <cmath>
#include <vector>
#include "config.h"

#define TILE_READABLE

using namespace std;

class Tile_T { // column-major
public:
    eid_t tile_id = -1;  // 16 PreEdge
    bool output = false;  // 1 PreEdge
    
    vid_t target_vertex_partition_id = -1;  // 16 to PreEdge
    vid_t target_vertex_begin = -1;  // 32
    int target_vertex_total = 0;  // 16 to PreEdge
    int source_vertex_total = 0;   // 16 to PreEdge

    vector<vid_t> source_vertex_list;  // 32
    vector<int> forward_edge_type;  // 16 to PreEdge
    vector<eid_t> forward_edge_idx;  // 16 to PreEdge
    vector<vid_t> forward_source_idx;  // 16 to PreEdge
    vector<vid_t> forward_target_idx;  // 16 to PreEdge
    
    int forward_edge_total = 0;  // 16 to PreEdge
    // vector<int> backward_edge_idx;  // 16 to PreEdge
    // vector<int> backward_edge_type;  // 16 to PreEdge
    // vector<int> backward_target_idx;  // 16 to PreEdge
    // vector<int> backward_source_idx;  // 16 to PreEdge
    // int backward_edge_total;  // 16 to PreEdge
    
    long num_bytes_scalar_to_load = ceil(
            (2 + 1.0 / 8) + (2 * 4));
    
    ~Tile_T() {
#ifndef TILE_READABLE
        delete[] forward_edge_idx;
        delete[] forward_edge_type;
        delete[] forward_source_idx;
        delete[] forward_target_idx;
        delete[] source_vertex_list;
#else
        forward_edge_idx.clear();
        forward_edge_type.clear();
        forward_source_idx.clear();
        forward_target_idx.clear();
        source_vertex_list.clear();
#endif
    }
    
    inline bool check_valid() const {

#ifndef TILE_READABLE
        if (source_vertex_list == nullptr) return false;
        if (forward_edge_type == nullptr) return false;
        if (forward_edge_idx == nullptr) return false;
        if (forward_source_idx == nullptr) return false;
        if (forward_target_idx == nullptr) return false;
#else
        if (source_vertex_list.empty()) return false;
        if (forward_edge_type.empty()) return false;
        if (forward_edge_idx.empty()) return false;
        if (forward_source_idx.empty()) return false;
        if (forward_target_idx.empty()) return false;
#endif
        
        if (forward_edge_total == 0) return false;
        if (target_vertex_total == 0) return false;
        if (source_vertex_total == 0) return false;
        return true;
    }
};

#endif //CPPMAIN_TILE_H
