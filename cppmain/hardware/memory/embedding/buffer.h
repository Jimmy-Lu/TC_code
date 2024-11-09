//
// Created by SmartCabbage on 2021/7/23.
//

#ifndef CPPMAIN_BUFFER_H
#define CPPMAIN_BUFFER_H

#include "../../message/message.h"
#include <deque>

class Buffer {
protected:
    // MemoryAccessHandler *mah;
    
    size_t size; // byte
    long long clk = 0;
    long cache_read_access = 0;
    long cache_write_access = 0;
    long cache_total_access = 0;
    int cache_cycle_access = 0;
    int latency; // 1 / 3.2 * 31 = 9.6875 ns
    
    std::deque<std::pair<long, RequestBuffer *> > hit_list;

public:
    explicit Buffer(int size);
    
    void tick();
    bool send(RequestBuffer *req);
    // double get_energy_dynamic_mJ() const;
    
    enum {
        READY, BUSY,
    } state = READY;
};

class BufferGroup {
protected:
    vector<Buffer *> buffers;
    
    // const int byte_per_access = 128; // 32*4
    long long access_total_read = 0;
    long long access_total_write = 0;

public:
    BufferGroup() {
        buffers.assign(Config::num_thread, new Buffer(Config::SIZE_SRC_MEMORY_MB / Config::num_thread));
        buffers.push_back(new Buffer(Config::SIZE_SRC_MEMORY_MB / Config::num_thread));
    }
    
    ~BufferGroup() {
        for (auto b: buffers) delete b;
        buffers.clear();
    }
    
    inline void send(RequestBuffer *req) {
        auto sid = req->thread_id;
        buffers[sid]->send(req);
    }
    
    inline bool check_state_busy(int sid) const { return buffers[sid]->state == Buffer::BUSY; }
    inline bool check_state_ready(int sid) const { return buffers[sid]->state == Buffer::READY; }
    inline void set_state_busy(int sid) { buffers[sid]->state = Buffer::BUSY; }
    inline void set_state_ready(int sid) { buffers[sid]->state = Buffer::READY; }
    inline long long get_num_accesses_total() const { return access_total_read + access_total_write; }
    inline void add_access(const num_access_t &accesses) {
        access_total_read += (long long) (get<0>(accesses));
        access_total_write += (long long) (get<1>(accesses));
    }
};

#endif //CPPMAIN_BUFFER_H
