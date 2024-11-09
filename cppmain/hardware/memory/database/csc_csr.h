//
// Created by flowe on 2021/8/24.
//

#ifndef CPPMAIN_CSC_CSR_H
#define CPPMAIN_CSC_CSR_H

#include <numeric>
#include <experimental/filesystem>
#include "../../../configs/tile.h"

class CSC_CSR {
public:
    
    vector<eid_t> csc_ptr_col;
    vector<vid_t> csc_idx_row;
    vector<eid_t> csr_ptr_row;
    vector<eid_t> csr_max_row;
    vector<vid_t> csr_cnt_row_scan;
    vector<vid_t> csr_cnt_row_generate;
    vector<vid_t> csr_idx_col;
    vector<eid_t> csc_idx_edge;
    
    long long addr_vertex_begin;
    long long addr_final_begin;
    
    eid_t num_edges = -1;
    vid_t num_vertices = -1;
    vid_t num_partitions = -1;
    string iFile;
    
    CSC_CSR() {
        iFile = dict_source_dir.at(Config::name_dataset);
        cout << "Input file: " << iFile << endl;
        
        load_adj_csc();
        sort_adj_csc();
        load_adj_csr();
        sort_adj_csr();
        
        addr_vertex_begin = 0;
        addr_final_begin = num_vertices * Config::SIZE_FEATURE_BYTE + addr_vertex_begin;
        num_partitions = ceil(double(num_vertices) / Config::tile_num_dst);
        
        printf("---- Graph Property ----\n");
        printf("  # Vertex:  %d\n", num_vertices);
        printf("  #  Edge:   %d\n", num_edges);
        printf("------------------------\n");
    }

protected:
    void load_adj_csc();
    void load_adj_csr();
    void sort_adj_csc();
    void sort_adj_csr();
    
};

#endif //CPPMAIN_CSC_CSR_H
