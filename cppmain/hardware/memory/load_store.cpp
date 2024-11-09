//
// Created by Flowerbeach on 2021/7/25.
//

#include "load_store.h"

// todo csc/csr multi-channel access
long LoadStoreUnit::read_csc_ptr(RequestLSU *req) {
    int num_elements = req->cb_args->tag.at("len_ptr");
    vid_t offset = req->cb_args->tag.at("begin_ptr");
    long access_bytes = num_elements * size_element;
    (*req->sram).assign(num_elements, -1);
    if (offset + num_elements > num_vertices + 1) num_elements = num_vertices + 1 - offset;
    for (int i = 0; i < num_elements; i++)
        (*req->sram)[i] = csc_csr->csc_ptr_col[i + offset];
    callback_args_t callback_arg;
    long long addr = addr_csc_ptr_begin + offset * size_element;
    mem_access_adj(req, callback_arg, addr, access_bytes);
    return access_bytes;
}

long LoadStoreUnit::write_csr_cnt(RequestLSU *req) {
    int num_elements = req->cb_args->tag.at("len_ptr");
    vid_t offset = req->cb_args->tag.at("begin_ptr");
    
    if (offset < 0) {
        callback_args_t callback_arg;
        req->callback(callback_arg);
        delete req;
        return 0;
        
    } else {
        long access_bytes = num_elements * size_element;
        assert(Config::bytes_dram_access_channel == access_bytes);
        
        // select csr_cnt_row
        vector<vid_t> *csr_cnt_row;
        if (req->source_device == "GenerateCsrOnly")
            csr_cnt_row = &csc_csr->csr_cnt_row_generate;
        else csr_cnt_row = &csc_csr->csr_cnt_row_scan;
        
        // copy data to the buffer
        num_elements = num_elements / 2 - 1;
        if (offset + num_elements > num_vertices) num_elements = num_vertices - offset;
        for (int i = 0; i < num_elements; i++)
            (*csr_cnt_row)[i + offset] = (*req->sram)[i * 2 + 1];
        
        // generate dram transactions
        tag_t tag = {{"write_csr_cnt", 1}};
        callback_args_t callback_arg = callback_args_t(tag);
        long long addr = addr_csr_cnt_begin + offset * size_element;
        mem_access_adj(req, callback_arg, addr, access_bytes);
        return access_bytes;
    }
}

long LoadStoreUnit::read_csr_cnt(RequestLSU *req) {
    int num_elements = req->cb_args->tag.at("len_ptr");
    vid_t offset = req->cb_args->tag.at("begin_ptr");
    assert(offset >= 0);
    
    long access_bytes = num_elements * size_element;
    assert(Config::bytes_dram_access_channel == access_bytes);
    (*req->sram).assign(num_elements, 0);
    
    // select csr_cnt_row
    vector<vid_t> *csr_cnt_row;
    if (req->source_device == "GenerateCsrOnly")
        csr_cnt_row = &csc_csr->csr_cnt_row_generate;
    else csr_cnt_row = &csc_csr->csr_cnt_row_scan;
    
    // copy data to the buffer
    num_elements = num_elements / 2 - 1;
    if (offset + num_elements > num_vertices) num_elements = num_vertices - offset;
    for (int i = 0; i < num_elements; i++)
        (*req->sram)[i * 2 + 1] = (*csr_cnt_row)[i + offset];
    for (int i = 0; i < num_elements + 1; i++)
        (*req->sram)[i * 2] = csc_csr->csr_ptr_row[i + offset];
    
    // generate dram transactions
    tag_t tag = {{"read_csr_cnt", 1}};
    callback_args_t callback_arg = callback_args_t(tag);
    long long addr = addr_csr_cnt_begin + offset * size_element;
    mem_access_adj(req, callback_arg, addr, access_bytes);
    return access_bytes;
}

long LoadStoreUnit::write_csr_max(RequestLSU *req) {
    int num_elements = req->cb_args->tag.at("len_max");
    vid_t offset = req->cb_args->tag.at("write_max");
    
    if (offset < 0) {
        callback_args_t callback_arg;
        req->callback(callback_arg);
        delete req;
        return 0;
        
    } else {
        long access_bytes = num_elements * size_element;
        // assert(Config::SIZE_ReqDRAM_BYTE == access_bytes);
        
        // copy data to the buffer
        if (offset + num_elements > num_vertices) num_elements = num_vertices - offset;
        for (int i = 0; i < num_elements; i++)
            csc_csr->csr_max_row[i + offset] = (*req->sram)[i];
        
        // generate dram transactions
        tag_t tag = {{"write_csr_max", 1}};
        callback_args_t callback_arg = callback_args_t(tag);
        long long addr = addr_csr_cnt_begin + offset * size_element;
        mem_access_adj(req, callback_arg, addr, access_bytes);
        return access_bytes;
    }
}

long LoadStoreUnit::read_csr_max(RequestLSU *req) {
    int num_elements = req->cb_args->tag.at("len_max");
    vid_t offset = req->cb_args->tag.at("read_max");
    assert(offset >= 0);
    
    long access_bytes = num_elements * size_element;
    // assert(Config::SIZE_ReqDRAM_BYTE == access_bytes);
    (*req->sram).assign(num_elements, 0);
    
    // copy data to the buffer
    if (offset + num_elements > num_vertices) num_elements = num_vertices - offset;
    for (int i = 0; i < num_elements; i++)
        (*req->sram)[i] = csc_csr->csr_max_row[i + offset];
    
    // generate dram transactions
    tag_t tag = {{"read_csr_max", 1}};
    callback_args_t callback_arg = callback_args_t(tag);
    long long addr = addr_csr_cnt_begin + offset * size_element;
    mem_access_adj(req, callback_arg, addr, access_bytes);
    return access_bytes;
}

long LoadStoreUnit::read_csr_idx(RequestLSU *req) {
    int num_elements = req->cb_args->tag.at("len_idx");
    eid_t offset = req->cb_args->tag.at("begin_idx");
    long access_bytes = num_elements * size_element;
    assert(Config::bytes_dram_access_channel == access_bytes);
    for (int i = 0; i < num_elements; i++)
        (*req->sram)[i] = csc_csr->csr_idx_col[i + offset];
    tag_t tag = {{"read_csr_idx", 1}};
    callback_args_t callback_arg = callback_args_t(tag);
    long long addr = addr_csr_idx_begin + offset * size_element;
    mem_access_adj(req, callback_arg, addr, access_bytes);
    return access_bytes;
}

long LoadStoreUnit::read_csc_idx(RequestLSU *req) {
    int num_elements = req->cb_args->tag.at("len_idx");
    eid_t offset = req->cb_args->tag.at("begin_idx");
    long access_bytes = num_elements * size_element;
    assert(Config::bytes_dram_access_channel == access_bytes);
    for (int i = 0; i < num_elements; i++)
        (*req->sram)[i] = csc_csr->csc_idx_row[i + offset];
    tag_t tag = {{"col_idx",      req->cb_args->tag.at("col_idx")},
                 {"target_state", req->cb_args->tag.at("target_state")},
                 {"read_csc_idx", 1}};
    callback_args_t callback_arg = callback_args_t(tag);
    long long addr = addr_csc_idx_begin + offset * size_element;
    mem_access_adj(req, callback_arg, addr, access_bytes);
    return access_bytes;
}

long LoadStoreUnit::read_bitmap(RequestLSU *req) {
    int level = req->cb_args->tag.at("level");
    int id_x = req->cb_args->tag.at("id_x");
    int id_y = req->cb_args->tag.at("id_y");
    long access_bytes = Config::bytes_dram_access_channel;
    *(req->bitmap) = bitmap->bitmap->levels[level][id_x][id_y];
    callback_args_t callback_arg = callback_args_t(tag_t());
    long long addr = (addr_bitmap_begin + (long long) (Config::bytes_dram_access_channel)
                                          * (*req->bitmap)->id_linear) % Config::bytes_dram_capacity;
    mem_access_adj(req, callback_arg, addr, access_bytes);
    return access_bytes;
}

long LoadStoreUnit::read_adj(RequestLSU *req) {
    long access_bytes = 0;
    auto tid = req->cb_args->tag.at("tile_id");
    if (check_invalid_tid(tid)) {
        tag_t tag = {{"tile", -1}};
        callback_args_t callback_arg = callback_args_t(tag);
        req->callback(callback_arg);
        delete req;
        
    } else {
        auto *tile = tile_holder->get_tile_i(tid);
        tile->tile_id = tid;
        callback_args_t callback_arg({{"tile", 1}}, tile);
        access_bytes =
                tile->num_bytes_scalar_to_load * 4
                + long(tile->source_vertex_total) * 4
                + long(tile->forward_edge_total) * 3 * 4;
        if (Config::name_model == EModel::ggnn
            || Config::name_model == EModel::rgcn) {
            access_bytes += long(tile->forward_edge_total) * 4;
        }
        
        long long addr = addr_tile_next;
        mem_access_adj(req, callback_arg, addr, access_bytes);
        addr_tile_next += access_bytes;
    }
    return access_bytes;
}

long LoadStoreUnit::read_src(RequestLSU *req) {
    auto tids = req->tile_ids;
    
    long access_bytes = 0;
    vector<vid_t> source_vertex_list;
    tag_t tag = {{"src", 1}};
    
    for (auto tid: tids) {
        auto *tile = get_tile_from_th(tid);
        assert(tile->tile_id == tid);
        access_bytes += tile->source_vertex_total * Config::SIZE_FEATURE_BYTE;
        tile->source_vertex_list.resize(tile->source_vertex_total);
        // assert(tile->source_vertex_list.size() == tile->source_vertex_total);
        source_vertex_list.insert(source_vertex_list.end(),
                                  tile->source_vertex_list.begin(),
                                  tile->source_vertex_list.end());
    }
    
    mem_access_embed(req, callback_args_t(tag),
                     &source_vertex_list[0], source_vertex_list.size());
    return access_bytes;
}

long LoadStoreUnit::read_dst(RequestLSU *req) {
    long access_bytes = 0;
    int pid = req->cb_args->tag.at("partition_id");
    if (pid >= num_partitions) {
        tag_t tag = {{"dst", -1}};
        callback_args_t callback_arg = callback_args_t(tag);
        req->callback(callback_arg);
        delete req;
        
    } else {
        tag_t tag = {{"dst", 1}};
        vid_t begin = pid * Config::tile_num_dst;
        vid_t end = (pid + 1) * Config::tile_num_dst;
        if (end > num_vertices) end = num_vertices;
        auto num_v = end - begin;
        access_bytes = (end - begin) * Config::SIZE_FEATURE_BYTE;
        
        auto *vids = new vid_t[num_v];
        parallel_for (vid_t i = 0; i < num_v; i++) {
            vids[i] = i + begin;
        }
        mem_access_embed(req, callback_args_t(tag), vids, num_v);
        delete[] vids;
    }
    return access_bytes;
}

long LoadStoreUnit::write_dst(RequestLSU *req) {
    vid_t pid = req->cb_args->tag.at("partition_id");
    assert(pid <= num_partitions);
    tag_t tag = {{"dst", 1}};
    
    vid_t begin = pid * Config::tile_num_dst;
    vid_t end = (pid + 1) * Config::tile_num_dst;
    if (end > num_vertices) end = num_vertices;
    auto num_v = end - begin;
    long access_bytes = (end - begin) * Config::SIZE_FEATURE_BYTE;
    
    auto *vids = new vid_t[num_v];
    parallel_for (vid_t i = 0; i < num_v; i++) vids[i] = i + begin;
    mem_access_embed(req, callback_args_t(tag), vids, num_v);
    delete[] vids;
    
    return access_bytes;
}

void LoadStoreUnit::mem_access_adj(
        RequestLSU *mreq, callback_args_t callback_arg, long long addr, long size_data) {
    int size_req = Config::bytes_dram_access_channel;
    vector<RequestDRAM *> dram_reqs;
    
    do {
        auto req = new RequestDRAM(
                addr, mreq->type_req,
                mreq->source_device);
        dram_reqs.push_back(req);
        addr += size_req;
        size_data -= size_req;
    } while (size_data > 0);
    
    assert(!dram_reqs.empty());
    dram_reqs.back()->set_callback(
            mreq->callback, std::move(callback_arg));
    queue_dram_reqs_graph.insert(
            queue_dram_reqs_graph.end(),
            std::make_move_iterator(dram_reqs.begin()),
            std::make_move_iterator(dram_reqs.end()));
    delete mreq;
}

void LoadStoreUnit::mem_access_embed(
        RequestLSU *mreq, callback_args_t callback_arg,
        const vid_t *vids, int len_vid) {
    int bytes_req = Config::bytes_dram_access_channel;
    assert (len_vid > 0);
    
    // todo extract the feature size from argument
    int SIZE_FEATURE_BYTE;
    long long addr_begin;
    if (mreq->type_req == TypeReqOp::READ) {
        SIZE_FEATURE_BYTE = Config::SIZE_FEATURE_BYTE;
        addr_begin = addr_vertex_begin;
    } else {
        SIZE_FEATURE_BYTE = Config::SIZE_FEATURE_BYTE;
        addr_begin = addr_final_begin;
    }
    
    int num_req_vertex = ceil(double(SIZE_FEATURE_BYTE) / bytes_req);
    vector<RequestDRAM *> dram_reqs(len_vid * num_req_vertex, nullptr);
    
    parallel_for (vid_t i = 0; i < len_vid; i++) {
        vid_t vid = vids[i];
        long long addr = addr_begin + vid * SIZE_FEATURE_BYTE;
        for (vid_t j = 0; j < num_req_vertex; j++) {
            auto req = new RequestDRAM(addr, mreq->type_req, mreq->source_device);
            dram_reqs[i * num_req_vertex + j] = req;
            addr += bytes_req;
        }
    }
    
    assert(!dram_reqs.empty());
    dram_reqs.back()->set_callback(
            mreq->callback, std::move(callback_arg));
    queue_dram_reqs_embed.insert(
            queue_dram_reqs_embed.end(),
            std::make_move_iterator(dram_reqs.begin()),
            std::make_move_iterator(dram_reqs.end()));
    delete mreq;
}
