//
// Created by Flowerbeach on 2021/3/28.
//

#include "../configs/shard.h"
#include "../configs/config.h"

#ifndef CPPMAIN_GRAPH_LOADER_H
#define CPPMAIN_GRAPH_LOADER_H

class COO;
class GraphLoader {
public:
    
    typedef deque<vid_t> adj_csc_v_t;
    typedef deque<eid_t> adj_csc_e_t;
    
    explicit GraphLoader();
    ~GraphLoader();
    void start_partitioning(int thread_id);
    
    experimental::filesystem::path path_partition;
    
    vector<adj_csc_v_t> adj_csc_v;
    vector<adj_csc_e_t> adj_csc_e;
    vector<vector<vid_t>> adj_csr_size;
    
    typedef tuple<vid_t, vid_t, vid_t> interval_t;
    interval_t *intervals = nullptr;
    
    eid_t num_edges = -1;
    vid_t num_vertices = -1;
    id_interval_t num_intervals = -1;
    
    vid_t empty_rows = 0;
    id_shard_t num_shards = 0;
    double buff_util_avg = 0;
    
    void load_adj();
    void create_folder();
    void generate_adj_e();
    void generate_intervals();
    
    static void save_coo(GraphLoader::adj_csc_v_t *adj, vid_t num_v);

protected:
    void sort_adj();
    void generate_adj(const COO *coo, const vector<vid_t> *new_ids);
    
};

#endif //CPPMAIN_GRAPH_LOADER_H
