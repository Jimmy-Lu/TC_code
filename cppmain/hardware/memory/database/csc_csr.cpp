//
// Created by flowe on 2021/8/24.
//

#include "csc_csr.h"

void CSC_CSR::sort_adj_csc() {
    Timer timer;
    timer.Start();
    auto begin = csc_idx_row.begin();
    {
        // for (uintE v = 0; v < num_v; ++v) {
        parallel_for (vid_t v = 0; v < num_vertices; ++v) {
            std::sort(begin + csc_ptr_col[v],
                      begin + csc_ptr_col[v + 1]);
        }
    }
    timer.Stop();
    timer.PrintSecond("ADJ Sorting Time");
}

void CSC_CSR::sort_adj_csr() {
    Timer timer;
    timer.Start();
    auto begin = csr_idx_col.begin();
    {
        // for (uintE v = 0; v < num_v; ++v) {
        parallel_for (vid_t v = 0; v < num_vertices; ++v) {
            std::sort(begin + csr_ptr_row[v],
                      begin + csr_ptr_row[v + 1]);
        }
    }
    timer.Stop();
    timer.PrintSecond("ADJ Sorting Time");
}

void CSC_CSR::load_adj_csc() {
    // read all content in the file to string
    ifstream infile(iFile + ".csc");
    if (infile.fail()) throw runtime_error("file open error.\n");
    std::stringstream buffer;
    buffer << infile.rdbuf();
    
    // initialize
    string token;
    
    // read vertex# and edge#
    getline(buffer, token, ' ');
    if (num_vertices == -1) num_vertices = vid_t(stol(token));
    else { assert(num_vertices == vid_t(stol(token))); }
    getline(buffer, token, '\n');
    if (num_edges == -1) num_edges = eid_t(stol(token));
    else { assert(num_edges == eid_t(stol(token))); }
    
    // read edge list to adj
    csc_ptr_col.resize(num_vertices + 1);
    csc_idx_row.resize(num_edges);
    csc_idx_edge.resize(num_edges);
    for (vid_t ii = 0; ii < num_vertices + 1; ii++) {
        getline(buffer, token, ' ');
        csc_ptr_col[ii] = vid_t(stol(token));
    }
    getline(buffer, token, '\n');
    
    eid_t eid_cnt = 0;
    for (eid_t ii = 0; ii < num_edges; ii++) {
        getline(buffer, token, ' ');
        csc_idx_row[ii] = vid_t(stol(token));
        csc_idx_edge[ii] = eid_cnt++;
    }
    getline(buffer, token, '\n');
    infile.close();
    buffer.clear();
}

void CSC_CSR::load_adj_csr() {
    // read all content in the file to string
    ifstream infile(iFile + ".csr");
    if (infile.fail()) throw runtime_error("file open error.\n");
    std::stringstream buffer;
    buffer << infile.rdbuf();
    
    // initialize
    string token;
    
    // read vertex# and edge#
    getline(buffer, token, ' ');
    if (num_vertices == -1) num_vertices = vid_t(stol(token));
    else { assert(num_vertices == vid_t(stol(token))); }
    getline(buffer, token, '\n');
    if (num_edges == -1) num_edges = eid_t(stol(token));
    else { assert(num_edges == eid_t(stol(token))); }
    
    // read edge list to adj
    csr_ptr_row.resize(num_vertices + 1);
    csr_max_row.resize(num_vertices, -1);
    csr_cnt_row_scan.resize(num_vertices, 0);
    csr_cnt_row_generate.resize(num_vertices, 0);
    csr_idx_col.resize(num_edges);
    for (vid_t ii = 0; ii < num_vertices + 1; ii++) {
        getline(buffer, token, ' ');
        csr_ptr_row[ii] = vid_t(stol(token));
    }
    getline(buffer, token, '\n');
    
    for (eid_t ii = 0; ii < num_edges; ii++) {
        getline(buffer, token, ' ');
        csr_idx_col[ii] = vid_t(stol(token));
    }
    getline(buffer, token, '\n');
    infile.close();
    buffer.clear();
}
