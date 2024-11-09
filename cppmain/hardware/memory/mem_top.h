//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_MEM_TOP_H
#define CPPMAIN_MEM_TOP_H

#include "../../configs/config.h"
#include "../message/message.h"

#include "load_store.h"
#include "embedding/buffer.h"
#include "ramulator/Memory.h"

class TileHub;

class MemoryAccessHandler {
public:
    BufferGroup *bg;
    LoadStoreUnit *lsu;
    Memory *mem;
    
    double arch_dram_frequency_coefficient;
    inline bool check_dram_to_tick(Memory *) const;

public:
    explicit MemoryAccessHandler();
    ~MemoryAccessHandler();
    inline void tick();
    inline void finish_ramulator();
    
    inline long long get_volume_data_read() const { return bytes_read; }
    inline long long get_volume_data_write() const { return bytes_write; }
    
    inline void set_th(TileHub *th) { lsu->set_th(th); }
    inline eid_t get_num_tiles() const { return lsu->get_num_tiles(); }
    inline eid_t get_num_edges() const { return lsu->get_num_edges(); }
    inline vid_t get_num_vertices() const { return lsu->get_num_vertices(); }
    inline vid_t get_num_partitions() const { return lsu->get_num_partitions(); }
    inline BufferGroup *get_bg() { return bg; }
    
    void issue_access(Op *op);
    void issue_access(RequestLSU *req);
    inline bool check_buffer_i_ready(int sid);
    inline bool check_end();

private:
    long long bytes_read = 0;
    long long bytes_write = 0;
    long long num_cycles = 0;
    
    inline callback_t get_cb_ldsrc(const function<void()> &stream_cb, int sid);
    inline callback_t get_cb_lddst(const function<void()> &stream_cb, int sid);
    inline callback_t get_cb_stdst(const function<void()> &stream_cb, int sid);
};

inline void MemoryAccessHandler::tick() {
    num_cycles++;
    while (check_dram_to_tick(mem)) {
        mem->tick();
    }
    lsu->tick();
}

inline void MemoryAccessHandler::finish_ramulator() { mem->finish(); }

inline bool MemoryAccessHandler::check_dram_to_tick(Memory *m) const {
    auto cycle_dram = (long long) (
            double(1 + m->get_num_cycle()) * arch_dram_frequency_coefficient);
    return cycle_dram < num_cycles;
    
}
inline bool MemoryAccessHandler::check_buffer_i_ready(int sid) {
    return bg->check_state_ready(sid);
}

inline bool MemoryAccessHandler::check_end() {
    if (mem->pending_requests() > 0) return false;
    return true;
}

inline callback_t MemoryAccessHandler::get_cb_ldsrc(
        const function<void()> &stream_cb, int sid) {
    return [this, stream_cb, sid](callback_args_t &msg_back) {
        assert(bg->check_state_busy(sid));
        assert(msg_back.tag.at("src") == 1);
        bg->set_state_ready(sid);
        stream_cb();
    };
}

inline callback_t MemoryAccessHandler::get_cb_lddst(
        const function<void()> &stream_cb, int sid) {
    return [this, stream_cb, sid](callback_args_t &msg_back) {
        assert(bg->check_state_busy(sid));
        assert(msg_back.tag.at("dst") == 1);
        bg->set_state_ready(sid);
        stream_cb();
    };
}

inline callback_t MemoryAccessHandler::get_cb_stdst(
        const function<void()> &stream_cb, int sid) {
    return [this, stream_cb, sid](callback_args_t &msg_back) {
        assert(bg->check_state_busy(sid));
        assert(msg_back.tag.at("dst") == 1);
        bg->set_state_ready(sid);
        stream_cb();
    };
}

#endif //CPPMAIN_MEM_TOP_H
