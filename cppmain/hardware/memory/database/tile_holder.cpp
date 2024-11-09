//
// Created by SmartCabbage on 2021/2/2.
//

#include <sstream>
#include <unistd.h>
#include "tile_holder.h"

TileHolder::TileHolder() {
    // check folder existence, create one if not.
    target_dir = experimental::filesystem::path("../../partition");
    target_dir /= string("dataset.") + dataset_to_string.at(Config::name_dataset);
    target_dir /= string("tiling.") + tiling_to_string.at(Config::name_tiling);
    if (Config::name_tiling == ETiling::sparse)
        target_dir += string(".") + reorder_to_string.at(Config::name_reorder);
    target_dir /= model_to_string.at(Config::name_model) + "_"
                  + to_string(Config::num_thread) + "_"
                  + to_string(Config::tile_num_dst) + "_"
                  + to_string(Config::SIZE_DST_MEMORY_MB) + "_"
                  + to_string(Config::SIZE_SRC_MEMORY_MB);
    while (experimental::filesystem::exists(target_dir / "lock.tile")) sleep(5);
    
    load_meta();
    load_tiles();
    
    // change here if running with other mappings.
    // addr_edge_begin = 0;
    addr_vertex_begin = 0;
    addr_final_begin = num_vertices * Config::SIZE_FEATURE_BYTE + addr_vertex_begin;
    addr_tile_begin = num_vertices * Config::SIZE_FEATURE_BYTE + addr_final_begin;
    num_partitions = vid_t(ceil(double(num_vertices) / Config::tile_num_dst));
}

void TileHolder::load_meta() {
    string line, path = target_dir / "meta.tile";
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

Tile_T *load_a_tile(EFormat tile_format,
                    const string &path) {
    ifstream infile_a, infile_b;
    auto *tile_i = new Tile_T();
    
    if (tile_format == EFormat::ascii) {
        string name, line;
        infile_a = ifstream(path.c_str());
        while (getline(infile_a, line)) {
            istringstream iss_name(line);
            iss_name >> name;
            getline(infile_a, line);
            istringstream iss_data(line);
            
            if (name == "tile_id") {
                iss_data >> tile_i->tile_id;
            } else if (name == "output") {
                iss_data >> tile_i->output;
            } else if (name == "target_vertex_partition_id") {
                iss_data >> tile_i->target_vertex_partition_id;
            } else if (name == "target_vertex_begin") {
                iss_data >> tile_i->target_vertex_begin;
            } else if (name == "target_vertex_total") {
                iss_data >> tile_i->target_vertex_total;
            } else if (name == "source_vertex_total") {
                iss_data >> tile_i->source_vertex_total;
            } else if (name == "source_vertex_list") {
                Config::ascii_2_vector(tile_i->source_vertex_list, iss_data, tile_i->source_vertex_total);
            } else if (name == "forward_edge_total") {
                iss_data >> tile_i->forward_edge_total;
            } else if (name == "forward_edge_idx") {
                Config::ascii_2_vector(tile_i->forward_edge_idx, iss_data, tile_i->forward_edge_total);
            } else if (name == "forward_edge_type") {
                Config::ascii_2_vector(tile_i->forward_edge_type, iss_data, tile_i->forward_edge_total);
            } else if (name == "forward_source_idx") {
                Config::ascii_2_vector(tile_i->forward_source_idx, iss_data, tile_i->forward_edge_total);
            } else if (name == "forward_target_idx") {
                Config::ascii_2_vector(tile_i->forward_target_idx, iss_data, tile_i->forward_edge_total);
            } else if (name == "num_bytes_scalar_to_load") {
                iss_data >> tile_i->num_bytes_scalar_to_load;
            } else throw runtime_error(string("tile loading: ") + name);
        }
    } else if (tile_format == EFormat::binary) {
        infile_b = ifstream(path.c_str(), ios::binary);
        
        //scalar
        Config::binary_2_string("tile_id", infile_b);
        Config::binary_2_scalar(tile_i->tile_id, infile_b);
        Config::binary_2_string("output", infile_b);
        Config::binary_2_scalar(tile_i->output, infile_b);
        Config::binary_2_string("target_vertex_partition_id", infile_b);
        Config::binary_2_scalar(tile_i->target_vertex_partition_id, infile_b);
        Config::binary_2_string("target_vertex_begin", infile_b);
        Config::binary_2_scalar(tile_i->target_vertex_begin, infile_b);
        Config::binary_2_string("target_vertex_total", infile_b);
        Config::binary_2_scalar(tile_i->target_vertex_total, infile_b);
        Config::binary_2_string("source_vertex_total", infile_b);
        Config::binary_2_scalar(tile_i->source_vertex_total, infile_b);
        Config::binary_2_string("forward_edge_total", infile_b);
        Config::binary_2_scalar(tile_i->forward_edge_total, infile_b);
        // Config::binary_2_string("backward_edge_total", infile_b);
        // Config::binary_2_scalar(tile_i->backward_edge_total, infile_b);
        Config::binary_2_string("num_bytes_scalar_to_load", infile_b);
        Config::binary_2_scalar(tile_i->num_bytes_scalar_to_load, infile_b);
        
        //vector
        Config::binary_2_string("source_vertex_list", infile_b);
        Config::binary_2_vector(tile_i->source_vertex_list, infile_b, tile_i->source_vertex_total);
        Config::binary_2_string("forward_edge_idx", infile_b);
        Config::binary_2_vector(tile_i->forward_edge_idx, infile_b, tile_i->forward_edge_total);
        Config::binary_2_string("forward_edge_type", infile_b);
        Config::binary_2_vector(tile_i->forward_edge_type, infile_b, tile_i->forward_edge_total);
        Config::binary_2_string("forward_source_idx", infile_b);
        Config::binary_2_vector(tile_i->forward_source_idx, infile_b, tile_i->forward_edge_total);
        Config::binary_2_string("forward_target_idx", infile_b);
        Config::binary_2_vector(tile_i->forward_target_idx, infile_b, tile_i->forward_edge_total);
        // //
        // Config::binary_2_string("backward_edge_idx", infile_b);
        // Config::binary_2_vector(tile_i->backward_edge_idx, infile_b);
        // Config::binary_2_string("backward_edge_type", infile_b);
        // Config::binary_2_vector(tile_i->backward_edge_type, infile_b);
        // Config::binary_2_string("backward_target_idx", infile_b);
        // Config::binary_2_vector(tile_i->backward_target_idx, infile_b);
        // Config::binary_2_string("backward_source_idx", infile_b);
        // Config::binary_2_vector(tile_i->backward_source_idx, infile_b);
        
        //close
        infile_b.close();
    } else throw runtime_error("EnumFormat\n");
    
    return tile_i;
}

void TileHolder::load_tiles() {
    ifstream infile;
    string path;
    
    int nt;
    vector<long> num_tile_proc;
    while (true) {
        // verify the number of tiles
        path = target_dir / (to_string(num_tile_proc.size()) + ".check.tile");
        infile = ifstream(path.c_str());
        if (!infile) break;
        infile >> nt;
        infile.close();
        
        num_tile_total += nt;
        num_tile_proc.push_back(nt);
    }
    
    for (int np = 0; np < num_tile_proc.size(); np++) {
        for (long i = 0; i < num_tile_proc[np]; i++) {
            path = target_dir / (to_string(np) + "." + to_string(i) + ".tile");
            auto tile_i = load_a_tile(Config::tile_format, path);
            tiles.push_back(tile_i);
        }
    }
}

Tile_T *TileHolder::get_tile_i(eid_t tid) const {
    assert(tid < tiles.size());
    assert(tiles[tid] != nullptr);
    return tiles[tid];
}
