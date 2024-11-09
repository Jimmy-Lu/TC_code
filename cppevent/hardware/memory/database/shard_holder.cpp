//
// Created by SmartCabbage on 2021/2/2.
//

#include <sstream>
#include <unistd.h>
#include "shard_holder.h"

ShardHolder::ShardHolder() {
    // check folder existence, create one if not.
    target_dir = experimental::filesystem::path("../../partition");
    target_dir /= string("dataset.") + dataset_to_string.at(Config::name_dataset);
    target_dir /= string("partitioning.") + partitioning_to_string.at(Config::name_partitioning);
    if (Config::name_partitioning == EPartitioning::sparse)
        target_dir += string(".") + reorder_to_string.at(Config::name_reorder);
    target_dir /= model_to_string.at(Config::name_model) + "_"
                  + to_string(Config::num_thread) + "_"
                  + to_string(Config::shard_max_dst) + "_"
                  + to_string(Config::SIZE_DST_MEMORY_MB) + "_"
                  + to_string(Config::SIZE_SRC_MEMORY_MB);
    while (experimental::filesystem::exists(target_dir / "lock.shard")) sleep(5);
    
    load_meta();
    load_shards();
    
    // change here if running with other mappings.
    // addr_edge_begin = 0;
    addr_vertex_begin = 0;
    addr_final_begin = num_vertices * Config::SIZE_FEATURE_BYTE + addr_vertex_begin;
    addr_shard_begin = num_vertices * Config::SIZE_FEATURE_BYTE + addr_final_begin;
    num_intervals = id_interval_t(ceil(double(num_vertices) / Config::shard_max_dst));
}

void ShardHolder::load_meta() {
    string line, path = target_dir / "meta.shard";
    ifstream infile(path.c_str());
    if (!infile) throw runtime_error("load_meta\n");
    getline(infile, line);
    istringstream iss_vs(line);
    iss_vs >> num_edges >> num_vertices;
    assert(num_vertices > 0);
    assert(num_edges > 0);
    
    //
    vid_t degree;
    getline(infile, line);
    istringstream iss_degree(line);
    degree_vertex.resize(num_vertices);
    for (vid_t i = 0; i < num_vertices; i++) {
        iss_degree >> degree;
        degree_vertex[i] = degree;
    }
    
    printf("---- Graph Property ----\n");
    printf("  # Vertex:  %d\n", num_vertices);
    printf("  #  Edge:   %d\n", num_edges);
    printf("------------------------\n");
}

Shard_T *load_a_shard(EFormat shard_format,
                      const string &path) {
    ifstream infile_a, infile_b;
    auto *shard_i = new Shard_T();
    
    if (shard_format == EFormat::ascii) {
        string name, line;
        infile_a = ifstream(path.c_str());
        while (getline(infile_a, line)) {
            istringstream iss_name(line);
            iss_name >> name;
            getline(infile_a, line);
            istringstream iss_data(line);
            
            if (name == "shard_id") {
                iss_data >> shard_i->shard_id;
            } else if (name == "output") {
                iss_data >> shard_i->output;
            } else if (name == "target_interval_id") {
                iss_data >> shard_i->target_interval_id;
            } else if (name == "target_vertex_begin") {
                iss_data >> shard_i->target_vertex_begin;
            } else if (name == "target_vertex_total") {
                iss_data >> shard_i->target_vertex_total;
            } else if (name == "source_vertex_total") {
                iss_data >> shard_i->source_vertex_total;
            } else if (name == "source_vertex_list") {
                Config::ascii_2_vector(shard_i->source_vertex_list, iss_data, shard_i->source_vertex_total);
            } else if (name == "forward_edge_total") {
                iss_data >> shard_i->forward_edge_total;
            } else if (name == "forward_edge_idx") {
                Config::ascii_2_vector(shard_i->forward_edge_idx, iss_data, shard_i->forward_edge_total);
            } else if (name == "forward_edge_type") {
                Config::ascii_2_vector(shard_i->forward_edge_type, iss_data, shard_i->forward_edge_total);
            } else if (name == "forward_source_idx") {
                Config::ascii_2_vector(shard_i->forward_source_idx, iss_data, shard_i->forward_edge_total);
            } else if (name == "forward_target_idx") {
                Config::ascii_2_vector(shard_i->forward_target_idx, iss_data, shard_i->forward_edge_total);
            } else if (name == "num_bytes_scalar_to_load") {
                iss_data >> shard_i->num_bytes_scalar_to_load;
            } else throw runtime_error(string("shard loading: ") + name);
        }
    } else if (shard_format == EFormat::binary) {
        infile_b = ifstream(path.c_str(), ios::binary);
        
        //scalar
        Config::binary_2_string("shard_id", infile_b);
        Config::binary_2_scalar(shard_i->shard_id, infile_b);
        Config::binary_2_string("output", infile_b);
        Config::binary_2_scalar(shard_i->output, infile_b);
        Config::binary_2_string("target_interval_id", infile_b);
        Config::binary_2_scalar(shard_i->target_interval_id, infile_b);
        Config::binary_2_string("target_vertex_begin", infile_b);
        Config::binary_2_scalar(shard_i->target_vertex_begin, infile_b);
        Config::binary_2_string("target_vertex_total", infile_b);
        Config::binary_2_scalar(shard_i->target_vertex_total, infile_b);
        Config::binary_2_string("source_vertex_total", infile_b);
        Config::binary_2_scalar(shard_i->source_vertex_total, infile_b);
        Config::binary_2_string("forward_edge_total", infile_b);
        Config::binary_2_scalar(shard_i->forward_edge_total, infile_b);
        // Config::binary_2_string("backward_edge_total", infile_b);
        // Config::binary_2_scalar(shard_i->backward_edge_total, infile_b);
        Config::binary_2_string("num_bytes_scalar_to_load", infile_b);
        Config::binary_2_scalar(shard_i->num_bytes_scalar_to_load, infile_b);
        
        //vector
        Config::binary_2_string("source_vertex_list", infile_b);
        Config::binary_2_vector(shard_i->source_vertex_list, infile_b, shard_i->source_vertex_total);
        Config::binary_2_string("forward_edge_idx", infile_b);
        Config::binary_2_vector(shard_i->forward_edge_idx, infile_b, shard_i->forward_edge_total);
        Config::binary_2_string("forward_edge_type", infile_b);
        Config::binary_2_vector(shard_i->forward_edge_type, infile_b, shard_i->forward_edge_total);
        Config::binary_2_string("forward_source_idx", infile_b);
        Config::binary_2_vector(shard_i->forward_source_idx, infile_b, shard_i->forward_edge_total);
        Config::binary_2_string("forward_target_idx", infile_b);
        Config::binary_2_vector(shard_i->forward_target_idx, infile_b, shard_i->forward_edge_total);
        // //
        // Config::binary_2_string("backward_edge_idx", infile_b);
        // Config::binary_2_vector(shard_i->backward_edge_idx, infile_b);
        // Config::binary_2_string("backward_edge_type", infile_b);
        // Config::binary_2_vector(shard_i->backward_edge_type, infile_b);
        // Config::binary_2_string("backward_target_idx", infile_b);
        // Config::binary_2_vector(shard_i->backward_target_idx, infile_b);
        // Config::binary_2_string("backward_source_idx", infile_b);
        // Config::binary_2_vector(shard_i->backward_source_idx, infile_b);
        
        //close
        infile_b.close();
    } else throw runtime_error("EnumFormat\n");
    
    return shard_i;
}

void ShardHolder::load_shards() {
    ifstream infile;
    string path;
    
    int nt;
    vector<long> num_shard_proc;
    while (true) {
        // verify the number of shards
        path = target_dir / (to_string(num_shard_proc.size()) + ".check.shard");
        infile = ifstream(path.c_str());
        if (!infile) break;
        infile >> nt;
        infile.close();
        
        num_shard_total += nt;
        num_shard_proc.push_back(nt);
    }
    
    for (int np = 0; np < num_shard_proc.size(); np++) {
        for (long i = 0; i < num_shard_proc[np]; i++) {
            path = target_dir / (to_string(np) + "." + to_string(i) + ".shard");
            auto shard_i = load_a_shard(Config::shard_format, path);
            shards.push_back(shard_i);
        }
    }
}

Shard_T *ShardHolder::get_shard_i(eid_t tid) const {
    assert(tid < shards.size());
    assert(shards[tid] != nullptr);
    return shards[tid];
}
