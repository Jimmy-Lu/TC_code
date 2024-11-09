//
// Created by Flowerbeach on 2021/7/25.
//

#include "load_store.h"

long LoadStoreUnit::read_adj(RequestLSU *req) {
    long access_bytes = 0;
    id_shard_t sid = req->cb_args->tag.at("shard_id");
    if (check_invalid_sid(sid)) {
        tag_t tag = {{"shard", -1}};
        callback_args_t callback_arg = callback_args_t(tag);
        req->callback(callback_arg);
        delete req;
        
    } else {
        auto *shard = shard_holder->get_shard_i(sid);
        shard->shard_id = sid;
        callback_args_t callback_arg({{"shard", 1}}, shard);
        access_bytes =
                shard->num_bytes_scalar_to_load * 4
                + long(shard->source_vertex_total) * 4
                + long(shard->forward_edge_total) * 3 * 4;
        if (Config::name_model == EModel::ggnn
            || Config::name_model == EModel::rgcn) {
            access_bytes += long(shard->forward_edge_total) * 4;
        }
        
        long long addr = addr_shard_next;
        mem_access_adj(req, callback_arg, addr, access_bytes);
        addr_shard_next += access_bytes;
    }
    return access_bytes;
}

long LoadStoreUnit::read_src(RequestLSU *req) {
    auto sids = req->shard_ids;
    
    long access_bytes = 0;
    vector<vid_t> source_vertex_list;
    tag_t tag = {{"src", 1}};
    
    for (auto sid: sids) {
        auto *shard = get_shard_from_sh(sid);
        assert(shard->shard_id == sid);
        access_bytes += shard->source_vertex_total * Config::SIZE_FEATURE_BYTE;
        shard->source_vertex_list.resize(shard->source_vertex_total);
        // assert(shard->source_vertex_list.size() == shard->source_vertex_total);
        source_vertex_list.insert(source_vertex_list.end(),
                                  shard->source_vertex_list.begin(),
                                  shard->source_vertex_list.end());
    }
    
    mem_access_embed(req, callback_args_t(tag),
                     &source_vertex_list[0], source_vertex_list.size());
    return access_bytes;
}

long LoadStoreUnit::read_dst(RequestLSU *req) {
    long access_bytes = 0;
    id_interval_t iid = req->cb_args->tag.at("interval_id");
    if (iid >= num_intervals) {
        tag_t tag = {{"dst", -1}};
        callback_args_t callback_arg = callback_args_t(tag);
        req->callback(callback_arg);
        delete req;
        
    } else {
        tag_t tag = {{"dst", 1}};
        vid_t begin = iid * Config::shard_max_dst;
        vid_t end = (iid + 1) * Config::shard_max_dst;
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
    id_interval_t iid = req->cb_args->tag.at("interval_id");
    assert(iid <= num_intervals);
    tag_t tag = {{"dst", 1}};
    
    vid_t begin = iid * Config::shard_max_dst;
    vid_t end = (iid + 1) * Config::shard_max_dst;
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
