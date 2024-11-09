//
// Created by Flowerbeach on 2021/3/28.
//

#include <numeric>
#include <ctime>

#include "graph_loader.h"
#include "tile_generator.h"

#include "tiling/sparse.h"
#include "read_edgelist.h"

bool SAVE_ADJ_VIS = false;

GraphLoader::GraphLoader() {
    create_folder();
    load_adj();
    sort_adj();   // reorder the neighbour list of each vertex
    generate_adj_e();
    
    printf("---- Graph Property ----\n");
    printf("  # Vertex:  %d\n", num_vertices);
    printf("  #  Edge:   %d\n", num_edges);
    printf("------------------------\n");
}

GraphLoader::~GraphLoader() {
    for (vid_t i = 0; i < num_vertices; i++)
        assert(adj_csc_v[i].empty());
    delete[] partitions;
}

void GraphLoader::create_folder() {
    // check folder existence, create one if not.
    path_partition = fs::path("../../partition");
    if (!fs::exists(path_partition)) fs::create_directory(path_partition);
    path_partition /= string("dataset.") + dataset_to_string.at(Config::name_dataset);
    if (!fs::exists(path_partition)) fs::create_directory(path_partition);
    path_partition /= string("tiling.") + tiling_to_string.at(Config::name_tiling);
    path_partition += "." + reorder_to_string.at(Config::name_reorder);
    if (!fs::exists(path_partition)) fs::create_directory(path_partition);
    path_partition /= model_to_string.at(Config::name_model) + "_"
                      + to_string(Config::num_thread) + "_"
                      + to_string(Config::tile_num_dst) + "_"
                      + to_string(Config::SIZE_DST_MEMORY_MB) + "_"
                      + to_string(Config::SIZE_SRC_MEMORY_MB);
    if (!fs::exists(path_partition)) fs::create_directory(path_partition);
    
    // for remove error
    FILE *f = fopen((path_partition / "data.txt").c_str(), "w");
    assert(f != nullptr);
    fprintf(f, "%d\n", 0);
    fclose(f);
    
    // remove old tiling results in the folder.
    Config::path_trash /= to_string(std::time(nullptr));
    fs::rename(path_partition, Config::path_trash);
    fs::create_directory(path_partition);
}

void GraphLoader::generate_adj_e() {
    int edge_cnt = 0;
    adj_csc_e.resize(num_vertices);
    for (int dst = 0; dst < num_vertices; dst++) {
        adj_csc_e[dst].resize(adj_csc_v[dst].size());
        for (vid_t i = 0; i < adj_csc_v[dst].size(); i++) {
            adj_csc_e[dst][i] = edge_cnt++;
        }
    }
}

void GraphLoader::generate_partitions() {
    int size_partition_row = Config::tile_num_dst;
    num_partitions = ceil(double(num_vertices) / size_partition_row);
    partitions = new partition_t[num_partitions];
    for (int i = 0; i < num_partitions; i++) {
        partitions[i] = {i * size_partition_row, (i + 1) * size_partition_row, i};
        if ((i + 1) * size_partition_row >= num_vertices) break;
    }
    get<1>(partitions[num_partitions - 1]) = num_vertices;
    assert(num_vertices >= get<0>(partitions[num_partitions - 1]) && "last partition\n");
}

void GraphLoader::load_adj() {
    string iFile = dict_source_dir.at(Config::name_dataset) + ".coo";
    if (Config::name_reorder == EReorder::metis) iFile += ".metis_out";
    cout << "Input file: " << iFile << endl;
    
    Timer timer;
    timer.Start();
    /*************************************/
    vector<vid_t> num_in_degrees, num_out_degrees;
    auto *coo = load_coo(iFile, num_vertices, num_edges,
                         num_in_degrees, num_out_degrees);
    generate_partitions();
    /*************************************/
    timer.Stop();
    timer.PrintSecond("load_from_file");
    
    /*************************************/
    vector<vid_t> *new_ids = nullptr;
    if (Config::name_reorder != EReorder::metis
        && Config::name_reorder != EReorder::none) {
        new_ids = generate_mapping(num_vertices, num_edges,
                                   num_in_degrees, num_out_degrees);
    } // generate a reorder mapping
    // apply the reorder mapping
    generate_adj(coo, new_ids);
    delete coo;
    /*************************************/
    
    /*************************************/
    for (int i = 0; i < num_vertices; i++) num_in_degrees[i] = adj_csc_v[i].size();
    eid_t sum_degree = accumulate(num_in_degrees.begin(), num_in_degrees.end(), 0.0);
    assert(num_edges == sum_degree);
    fs::path target_path = path_partition / string("meta.tile");
    ofstream fin(target_path);
    fin << num_edges << " " << num_vertices << "\n";
    fin << Config::vector_2_string(num_in_degrees, num_vertices) << "\n";
    /*************************************/
    
    if (SAVE_ADJ_VIS) {
        save_coo(&adj_csc_v[0], num_vertices);
        cout << "adj_csc reordered generated.\n";
    }
}

double average(vector<double> const &v) {
    if (v.empty()) { return 0; }
    auto const count = static_cast<double >(v.size());
    return reduce(v.begin(), v.end()) / count;
}

void GraphLoader::start_partition(int thread_id) {
    TileGenerator *tg;
    if (Config::name_tiling == ETiling::sparse) {
        tg = new Sparse(this, thread_id);
    } else { abort(); }
    tg->generate_tiles();
    tg->end_partition();
    
    empty_rows = tg->get_empty_rows();
    num_tiles = tg->vec_util.size();
    buff_util_avg = average(tg->vec_util);
    
    delete tg;
}

void GraphLoader::save_coo(GraphLoader::adj_csc_v_t *adj, vid_t num_v) {
    auto target_path = fs::path("../../partition");
    target_path /= string("dataset.") + dataset_to_string.at(Config::name_dataset);
    assert(fs::exists(target_path));
    target_path /= string("tiling.") + tiling_to_string.at(Config::name_tiling);
    target_path += "." + reorder_to_string.at(Config::name_reorder);
    assert(fs::exists(target_path));
    target_path /= string("adj_csc.vis");
    cout << target_path << endl;
    
    string output;
    for (vid_t dst = 0; dst < num_v; dst++) {
        for (const auto &src: adj[dst])
            output += (to_string(src) + " " + to_string(dst) + "\n");
    }
    ofstream outfile_a(target_path);
    outfile_a << output;
    outfile_a.close();
}

void GraphLoader::generate_adj(const COO *coo, const vector<vid_t> *new_ids) {
    Timer timer;
    timer.Start();
    // make a new vertex list for the new graph
    // and map vertices and generate the new graph
    // if (Config::name_tiling == ETiling::sparse) {
    adj_csc_v.resize(num_vertices);
    adj_csc_e.resize(num_vertices);
    adj_csr_size.resize(num_vertices);
    
    for (vid_t v_id = 0; v_id < num_vertices; v_id++) {
        adj_csr_size[v_id].resize(num_partitions);
        for (int p_id = 0; p_id < num_partitions; p_id++)
            adj_csr_size[v_id][p_id] = 0;
    }
    
    //count csr dst_list size
    vid_t src, dst;
    for (int e = 0; e < num_edges; e++) {
        if (new_ids) {
            src = (*new_ids)[coo->src[e]];
            dst = (*new_ids)[coo->dst[e]];
        } else {
            src = coo->src[e];
            dst = coo->dst[e];
        }
        assert(src < num_vertices && dst < num_vertices);
        adj_csr_size[src][dst / Config::tile_num_dst]++;
        adj_csc_v[dst].push_back(src);
        adj_csc_e[dst].push_back(e);
    }
    // }
    
    timer.Stop();
    timer.PrintSecond("generate_adj");
}

void GraphLoader::sort_adj() {
    Timer timer;
    timer.Start();
    {
        // for (uintE v = 0; v < num_v; ++v) {
        for (uintE v = 0; v < num_vertices; ++v) {
            auto begin = adj_csc_v[v].begin();
            auto end = adj_csc_v[v].end();
            std::sort(begin, end);
        }
    }
    timer.Stop();
    timer.PrintSecond("sort_adj");
}

