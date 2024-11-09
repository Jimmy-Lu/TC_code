//
// Created by SmartCabbage on 2021/2/2.
//

#include "mem_top.h"

MemoryAccessHandler::MemoryAccessHandler() {
    auto *spec = new ramulator::HBM(Config::org, Config::speed);
    spec->set_channel_number(Config::num_channels);
    spec->set_rank_number(Config::num_ranks);
    vector<DramCtrl *> ctrls(Config::num_channels);
    for (int c = 0; c < Config::num_channels; c++) {
        auto *channel = new DRAM(spec, ramulator::HBM::Level::Channel);
        channel->id = c;
        channel->regStats("");
        auto *ctrl = new DramCtrl(channel);
        ctrls[c] = ctrl;
    }
    mem = new Memory(ctrls);
    lsu = new LoadStoreUnit(mem);
    bg = new BufferGroup();
    
    arch_dram_frequency_coefficient =
            double(Config::FrequencyArch_MHz) / mem->spec->speed_entry.freq / 2;
}

MemoryAccessHandler::~MemoryAccessHandler() {
    delete mem;
    delete lsu;
}

void MemoryAccessHandler::issue_access(RequestLSU *req) {
    long access_bytes;
    if (req->type_req == TypeReqOp::READ) {
        if (req->type_data == TypeData::adj) {
            access_bytes = lsu->read_adj(req);
        } else if (req->type_data == TypeData::csc_ptr) {
            access_bytes = lsu->read_csc_ptr(req);
        } else if (req->type_data == TypeData::csr_cnt_r) {
            access_bytes = lsu->read_csr_cnt(req);
        } else if (req->type_data == TypeData::csr_max_r) {
            access_bytes = lsu->read_csr_max(req);
        } else if (req->type_data == TypeData::csc_idx) {
            access_bytes = lsu->read_csc_idx(req);
        } else if (req->type_data == TypeData::csr_idx) {
            access_bytes = lsu->read_csr_idx(req);
        } else if (req->type_data == TypeData::bitmap) {
            access_bytes = lsu->read_bitmap(req);
        } else throw runtime_error("READ type not recognized.\n");
        bytes_read += access_bytes;
    } else if (req->type_req == TypeReqOp::WRITE) {
        if (req->type_data == TypeData::csr_cnt_w) {
            access_bytes = lsu->write_csr_cnt(req);
        } else if (req->type_data == TypeData::csr_max_w) {
            access_bytes = lsu->write_csr_max(req);
        } else throw runtime_error("WRITE type not recognized.\n");
        bytes_write += access_bytes;
    } else throw exception();
}

void MemoryAccessHandler::issue_access(Op *op) {
#ifdef _DEBUG_EXE
    cout << string("  issue to mah") + "\n";
#endif
    
    long access_bytes;
    auto req = new RequestLSU();
    req->thread_id = op->thread_id;
    req->source_device = string("Buffer") + to_string(op->thread_id);
    assert(bg->check_state_ready(op->thread_id));
    
    if (op->name == ISA::LD_DST) {
        assert(req->thread_id == Config::num_thread);
        tag_t tag = {{"partition_id", op->partition_id}};
        req->cb_args = new callback_args_t(tag);
        req->type_req = TypeReqOp::READ;
        req->type_data = TypeData::dst;
        req->callback = get_cb_lddst(op->callback, op->thread_id);
        access_bytes = lsu->read_dst(req);
        assert(bg->check_state_ready(op->thread_id));
        bg->set_state_busy(op->thread_id);
        bytes_read += access_bytes;
    } else if (op->name == ISA::LD_SRC) {
        tag_t tag = {{"tile_ids", -2}};
        req->cb_args = new callback_args_t(tag);
        req->tile_ids = op->tile_ids;
        req->callback = get_cb_ldsrc(op->callback, op->thread_id);
        req->type_req = TypeReqOp::READ;
        req->type_data = TypeData::src;
        access_bytes = lsu->read_src(req);
        assert(bg->check_state_ready(op->thread_id));
        bg->set_state_busy(op->thread_id);
        bytes_read += access_bytes;
    } else if (op->name == ISA::ST_DST) {
        tag_t tag = {{"partition_id", op->partition_id}};
        req->cb_args = new callback_args_t(tag);
        req->callback = get_cb_stdst(op->callback, op->thread_id);
        req->type_req = TypeReqOp::WRITE;
        req->type_data = TypeData::dst;
        access_bytes = lsu->write_dst(req);
        assert(bg->check_state_ready(op->thread_id));
        bg->set_state_busy(op->thread_id);
        bytes_write += access_bytes;
    } else throw runtime_error("WRITE type not recognized.\n");
}


