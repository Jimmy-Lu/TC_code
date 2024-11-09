//
// Created by Flowerbeach on 2021/3/28.
//

#include "sparse.h"
#include "algorithm"

void Sparse::generate_shards() {
    for (vid_t pid = thread_id; pid < gl->num_intervals; pid += Config::num_workers) {
        const auto cols_curr = gl->intervals[pid];
        vid_t row_curr = 0;
        
        while (row_curr < num_vertices) {
            select_rows(row_curr, cols_curr);
            if (shards.size() >= max_shard) save_shards();
        }
        
        // save shards to files
        if (!shards.empty()) {
            shards.back()->output = true;
            save_all_shards();
        } else continue;
    }
}

void Sparse::select_rows(vid_t &row_curr, const GraphLoader::interval_t &cols_curr) {
    auto[p_begin, p_end, pid] = cols_curr;
    auto *tile = new Shard_T();
    tile->output = false;
    tile->shard_id = num_shards;
    
    // destination
    tile->target_vertex_begin = p_begin;
    tile->target_vertex_total = p_end - p_begin;
    tile->target_interval_id = pid;
    
    // source
    tile->source_vertex_list.resize(Config::shard_max_src_edge);
    while (row_curr < num_vertices
           && (tile->forward_edge_total + gl->adj_csr_size[row_curr][pid]) * Config::num_element_feat_edge
              + (tile->source_vertex_total + 1) * Config::num_element_feat_src
              < Config::shard_max_element) {
        auto size = gl->adj_csr_size[row_curr][pid];
        if (size > 0) {
            tile->forward_edge_total += size;
            tile->source_vertex_list[tile->source_vertex_total++] = row_curr;
        } else empty_rows++;
        row_curr++;
    }
    if (tile->forward_edge_total == 0) {
        if (row_curr == num_vertices) {
            delete tile;
            return;
        } else {
            // only one row of edges will reach the shard size threshold
            assert(gl->adj_csr_size[row_curr][pid] * Config::num_element_feat_edge
                   + Config::num_element_feat_src > Config::shard_max_element);
            
            tile->forward_edge_total = int((Config::shard_max_element - Config::num_element_feat_src)
                                           / Config::num_element_feat_edge);
            gl->adj_csr_size[row_curr][pid] -= tile->forward_edge_total;
            tile->source_vertex_list[tile->source_vertex_total++] = row_curr;
        }
    }
    
    // edge list
    tile->forward_target_idx.resize(tile->forward_edge_total);
    tile->forward_source_idx.resize(tile->forward_edge_total);
    tile->forward_edge_idx.resize(tile->forward_edge_total);
    tile->forward_edge_type.resize(tile->forward_edge_total);
    
    // edge list
    int cnt_edge = 0;
    vid_t max = tile->source_vertex_list[tile->source_vertex_total - 1];
    for (int dst = p_begin; dst < p_end; dst++) {
        while (!gl->adj_csc_v[dst].empty()
               && gl->adj_csc_v[dst].front() <= max) {
            auto src = gl->adj_csc_v[dst].front();
            tile->forward_target_idx[cnt_edge] = dst;
            tile->forward_source_idx[cnt_edge] = src;
            tile->forward_edge_idx[cnt_edge] = gl->adj_csc_e[dst].front();
            tile->forward_edge_type[cnt_edge] = gl->adj_csc_e[dst].front()
                                                % Config::num_edge_types;
            gl->adj_csc_e[dst].pop_front();
            gl->adj_csc_v[dst].pop_front();
            cnt_edge++;
        }
        if (tile->source_vertex_total == 1
            && cnt_edge >= tile->forward_edge_total) { break; }
    }
    assert(cnt_edge == tile->forward_edge_total);
    
    // store to list
    shards.push_back(tile);
    double util = double(tile->forward_edge_total * Config::num_element_feat_edge
                         + tile->source_vertex_total * Config::num_element_feat_src)
                  / Config::shard_max_element;
    vec_util.push_back(util);
    num_shards += 1;
}

void Sparse::shard_saver(Shard_T *tile, EFormat tile_format,
                         const experimental::filesystem::path &target_path) {
    
    if (tile_format == EFormat::ascii) {
        ofstream outfile_a(target_path);
        //scalar
        outfile_a << "shard_id\n";
        outfile_a << tile->shard_id << "\n";
        outfile_a << "output\n";
        outfile_a << tile->output << "\n";
        outfile_a << "target_interval_id\n";
        outfile_a << tile->target_interval_id << "\n";
        outfile_a << "target_vertex_begin\n";
        outfile_a << tile->target_vertex_begin << "\n";
        outfile_a << "target_vertex_total\n";
        outfile_a << tile->target_vertex_total << "\n";
        outfile_a << "source_vertex_total\n";
        outfile_a << tile->source_vertex_total << "\n";
        outfile_a << "forward_edge_total\n";
        outfile_a << tile->forward_edge_total << "\n";
        // outfile_a << "backward_edge_total\n";
        // outfile_a << shard->backward_edge_total << "\n";
        outfile_a << "num_bytes_scalar_to_load\n";
        outfile_a << tile->num_bytes_scalar_to_load << "\n";
        
        // vector
        outfile_a << "source_vertex_list\n";
        outfile_a << tile->source_vertex_total << " "
                  << Config::vector_2_string(tile->source_vertex_list, tile->source_vertex_total).c_str() << "\n";
        outfile_a << "forward_edge_idx\n";
        outfile_a << tile->forward_edge_total << " "
                  << Config::vector_2_string(tile->forward_edge_idx, tile->forward_edge_total).c_str() << "\n";
        outfile_a << "forward_edge_type\n";
        outfile_a << tile->forward_edge_total << " "
                  << Config::vector_2_string(tile->forward_edge_type, tile->forward_edge_total).c_str() << "\n";
        outfile_a << "forward_source_idx\n";
        outfile_a << tile->forward_edge_total << " "
                  << Config::vector_2_string(tile->forward_source_idx, tile->forward_edge_total).c_str() << "\n";
        outfile_a << "forward_target_idx\n";
        outfile_a << tile->forward_edge_total << " "
                  << Config::vector_2_string(tile->forward_target_idx, tile->forward_edge_total).c_str() << "\n";
        
        //close
        outfile_a.close();
        
    } else if (tile_format == EFormat::binary) {
        ofstream outfile_b(target_path, ios::binary);
        
        //scalar
        Config::string_2_binary("shard_id", outfile_b);
        Config::scalar_2_binary(tile->shard_id, outfile_b);
        Config::string_2_binary("output", outfile_b);
        Config::scalar_2_binary(tile->output, outfile_b);
        Config::string_2_binary("target_interval_id", outfile_b);
        Config::scalar_2_binary(tile->target_interval_id, outfile_b);
        Config::string_2_binary("target_vertex_begin", outfile_b);
        Config::scalar_2_binary(tile->target_vertex_begin, outfile_b);
        Config::string_2_binary("target_vertex_total", outfile_b);
        Config::scalar_2_binary(tile->target_vertex_total, outfile_b);
        Config::string_2_binary("source_vertex_total", outfile_b);
        Config::scalar_2_binary(tile->source_vertex_total, outfile_b);
        Config::string_2_binary("forward_edge_total", outfile_b);
        Config::scalar_2_binary(tile->forward_edge_total, outfile_b);
        // Config::string_2_binary("backward_edge_total", outfile_b);
        // Config::scalar_2_binary(shard->backward_edge_total, outfile_b);
        Config::string_2_binary("num_bytes_scalar_to_load", outfile_b);
        Config::scalar_2_binary(tile->num_bytes_scalar_to_load, outfile_b);
        
        // vector
        Config::string_2_binary("source_vertex_list", outfile_b);
        Config::vector_2_binary(tile->source_vertex_list, outfile_b, tile->source_vertex_total);
        Config::string_2_binary("forward_edge_idx", outfile_b);
        Config::vector_2_binary(tile->forward_edge_idx, outfile_b, tile->forward_edge_total);
        Config::string_2_binary("forward_edge_type", outfile_b);
        Config::vector_2_binary(tile->forward_edge_type, outfile_b, tile->forward_edge_total);
        Config::string_2_binary("forward_source_idx", outfile_b);
        Config::vector_2_binary(tile->forward_source_idx, outfile_b, tile->forward_edge_total);
        Config::string_2_binary("forward_target_idx", outfile_b);
        Config::vector_2_binary(tile->forward_target_idx, outfile_b, tile->forward_edge_total);
        // //
        // Config::string_2_binary("backward_edge_idx", outfile_b);
        // Config::vector_2_binary(shard->backward_edge_idx, outfile_b);
        // Config::string_2_binary("backward_edge_type", outfile_b);
        // Config::vector_2_binary(shard->backward_edge_type, outfile_b);
        // Config::string_2_binary("backward_target_idx", outfile_b);
        // Config::vector_2_binary(shard->backward_target_idx, outfile_b);
        // Config::string_2_binary("backward_source_idx", outfile_b);
        // Config::vector_2_binary(shard->backward_source_idx, outfile_b);
        
        //close
        outfile_b.close();
        
    } else throw runtime_error("EnumFormat\n");
}
