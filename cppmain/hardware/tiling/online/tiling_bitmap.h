//
// Created by Flowerbeach on 2021/3/28.
//

#include "tiling_base.h"
#include "../../../configs/bitmap.h"

#ifndef CPPMAIN_TILING_BITMAP_H
#define CPPMAIN_TILING_BITMAP_H

class MemoryAccessHandler;

namespace tiling {
    class TilingUnitBitmap : public TilingUnit {
    public:
        TilingUnitBitmap(MemoryAccessHandler *mah, TileHubTU *th);
        
        void scan() override;
        inline double get_energy_mJ() const override {
            return double(num_cycles_busy) / double(Config::FrequencyArch_MHz * 1024 * 1024)
                   * Config::POWER_TU_BITMAP_mW;
        }
    private:
        void begin_level0() {
            // if (src_curr == 12671) {
            //     cout << endl;
            // }
            
            // re-activate scan-l1
            assert(scan_l1_state == 2);
            scan_l1_state = 0;
            
            // check if the target bitmap in the buffer
            dst_curr = partition_begin;
            vid_t x = src_curr / b_sizes[0].first;
            vid_t y = dst_curr / b_sizes[0].second;
            b_coos[0].first = src_curr % b_sizes[0].first;
            b_coos[0].second = dst_curr % b_sizes[0].second;
            if (b_ids[0].first != x || b_ids[0].second != y) {
                b_ids[0].first = x;
                b_ids[0].second = y;
                scan_l0_state = 1;
            } else { scan_l0_state = 2; }
        }
        void scan_level0() {
            assert(scan_l0_state == 2);
            
            for (int i = 0; i < scan_lens[0]; i++) {
                if (b_buffers[0]->check_bit_1(b_coos[0].first, b_coos[0].second + i)) {
                    if (b_ids[0].second * b_sizes[0].second
                        + (b_coos[0].second + i) * b_sizes[1].second < partition_end) {
                        scan_queue.emplace_back(
                                b_ids[0].first * Config::bitmap_sizes[0].first + b_coos[0].first,
                                b_ids[0].second * Config::bitmap_sizes[0].second + b_coos[0].second + i);
                    } else break;
                }
            }
            b_coos[0].second += scan_lens[0];
            if (b_ids[0].second * b_sizes[0].second
                + b_coos[0].second * b_sizes[1].second
                >= partition_end) {
                scan_l0_state = 3;
            } else if (b_coos[0].second
                       >= Config::bitmap_sizes[0].second) {
                b_ids[0].second++;
                scan_l0_state = 1;
            }
        }
        void scan_level1() {
            assert(scan_l1_state == 1);
            
            for (int i = 0; i < scan_lens[1]; i++) {
                if (b_buffers[1]->check_bit_1(b_coos[1].first, b_coos[1].second + i)) {
                    if (b_ids[1].second * b_sizes[1].second + (b_coos[1].second + i) < partition_end) {
                        // add a new edge to the tile
                        int target_pos_tile = t_num_edges + t_num_edges_of_src;
                        sram_tile->forward_edge_type[target_pos_tile] =
                                num_edges_counted % Config::num_edge_types;
                        sram_tile->forward_edge_idx[target_pos_tile] = num_edges_counted++;
                        sram_tile->forward_target_idx[target_pos_tile] =
                                b_ids[1].second * b_sizes[1].second + b_coos[1].second + i;
                        sram_tile->forward_source_idx[target_pos_tile] = src_curr;
                        t_num_edges_of_src++;
                    } else break;
                }
            }
            
            // check the fullness of the tile
            if ((t_num_srcs + 1) * Config::num_element_feat_src
                + (t_num_edges + t_num_edges_of_src) * Config::num_element_feat_edge
                > Config::num_total_element_tile) {
                t_num_edges_of_src = 0;
                scan_queue.clear();
                scan_l0_state = 0;
                scan_l1_state = 2;
                state = State::checkout;
                return;
            }
            
            // check finishing the current bitmap
            b_coos[1].second += scan_lens[1];
            if ((b_ids[1].second * b_sizes[1].second + b_coos[1].second >= partition_end) ||
                (b_coos[1].second >= Config::bitmap_sizes[1].second)) {
                scan_queue.pop_front();
                scan_l1_state = 0;
            }
        }
        void wait_level1_to_finish() {
            // whether l1-scan stopped
            if (scan_queue.empty() && scan_l1_state == 0) {
                scan_l0_state = 5;
                scan_l1_state = 2;
            }
        }
        void wait_level0_to_assign() {
            if (!scan_queue.empty()) {
                b_ids[1].first = scan_queue.front().first;
                b_ids[1].second = scan_queue.front().second;
                scan_l1_state = 3;
            }
        }
        void move_to_next_src() {
            assert(scan_l0_state == 5);
            assert(scan_l1_state == 2);
            
            // finalize the current row and move to the next
            if (t_num_edges_of_src > 0) {
                sram_tile->source_vertex_list[t_num_srcs++] = src_curr;
                t_num_edges += t_num_edges_of_src;
                t_num_edges_of_src = 0;
            } else { assert(t_num_edges_of_src == 0); }
            src_curr++;
            scan_l0_state = 0;
            
            if (src_curr >= src_end) state = State::partition;
        }
        void load_bitmap(int level);
    
    private:
        vector<pair<vid_t, vid_t>> b_ids;
        vector<pair<vid_t, vid_t>> b_sizes;
        deque<pair<vid_t, vid_t>> scan_queue;
        vector<BitmapSingle *> b_buffers;
        
        vector<int> scan_lens;
        vector<pair<int, int>> b_coos;
        
        int scan_l0_state = 0;
        int scan_l1_state = 2;
        vid_t dst_curr = -1;
    
    private:
        inline callback_t get_mah_callback(int level);
        void issue_to_mah(TypeData type, tag_t tag, callback_t cb) override;
    };
    
}

#endif //CPPMAIN_TILING_BITMAP_H
