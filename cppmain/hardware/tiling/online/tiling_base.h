//
// Created by Flowerbeach on 2021/3/28.
//

#include "../../../configs/tile.h"
#include "../../../configs/config.h"
#include "../../message/message.h"
#include "tile_hub_tu.h"

#ifndef CPPMAIN_TILING_BASE_H
#define CPPMAIN_TILING_BASE_H

class MemoryAccessHandler;

namespace tiling {
    class TilingUnit {
    public:
        string name;
    
    public:
        Tile_T *sram_tile = nullptr;
        vid_t partition_idx = -1;
        vid_t partition_begin = -1;
        vid_t partition_end = -1;
        int partition_size = -1;
        
        vid_t src_curr = -1;
        vid_t src_begin = -1;
        vid_t src_end = -1;
        
        int t_num_srcs = 0;
        int t_num_edges = 0;
        int t_num_edges_of_src = 0;
        
        enum class State {
            ready, scan, partition, checkout, end
        } state = State::ready;
    
    public:
        TilingUnit(MemoryAccessHandler *mah, TileHubTU *th);
        void tick();
        
        eid_t num_edges = -1;
        vid_t num_vertices = -1;
        vid_t num_partitions = -1;
        eid_t num_tiles_generated = 0;
        eid_t num_edges_counted = 0;
        
        inline bool check_state_end() const { return state == State::end; }
        inline eid_t get_num_tiles_total() const { return check_state_end() ? num_tiles_generated : -1; }
        
        inline void final_assert() const { assert(num_edges_counted == num_edges); }
        inline double get_utilization(long long cycles_total) const { return double(num_cycles_busy) / cycles_total; }
        inline virtual double get_energy_mJ() const = 0;
    
    protected:
        TileHubTU *th = nullptr;
        MemoryAccessHandler *mah = nullptr;
        deque<Tile_T *> tiles;
        
        virtual void scan() = 0;
        virtual void checkout();
        virtual void partition();
        virtual void begin(Tile_T *);
        
        // statistics
        long long num_cycles_busy = 0;
        long long num_cycles_idle = 0;
        long num_accesses = 0;
    
    protected:
        virtual void issue_to_mah(TypeData type, tag_t tag, callback_t cb) = 0;
        
        static void convert_from_csr_to_csc(Tile_T *t) {
            vector<int> dst_ptr(t->target_vertex_total, 0);
            for (int i = 0; i < t->forward_edge_total; i++)
                dst_ptr[t->forward_target_idx[i] - t->target_vertex_begin]++;
            int sum = 0, curr;
            for (int i = 0; i < t->target_vertex_total; i++) {
                curr = dst_ptr[i];
                dst_ptr[i] = sum;
                sum += curr;
            }
            assert(sum == t->forward_edge_total);
            
            auto forward_edge_idx = t->forward_edge_idx;
            auto forward_edge_type = t->forward_edge_type;
            auto forward_source_idx = t->forward_source_idx;
            auto forward_target_idx = t->forward_target_idx;
            for (int i = 0; i < t->forward_edge_total; i++) {
                auto pos = dst_ptr[forward_target_idx[i]
                                   - t->target_vertex_begin]++;
                t->forward_edge_idx[pos] = forward_edge_idx[i];
                t->forward_edge_type[pos] = forward_edge_type[i];
                t->forward_source_idx[pos] = forward_source_idx[i];
                t->forward_target_idx[pos] = forward_target_idx[i];
            }
        }
    };
}

#endif //CPPMAIN_TILING_BASE_H
