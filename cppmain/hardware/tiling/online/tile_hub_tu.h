//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_TILE_HUB_TU_H
#define CPPMAIN_TILE_HUB_TU_H

#include "../offline/tile_hub.h"

class TileHubTU : public TileHub {
public:
    explicit TileHubTU() : TileHub(nullptr) {};
    
    inline void tick() override {};
    inline void set_position_end();
    inline Tile_T *get_tile_tiling();
    inline bool check_new_position_tiling();
    inline void receive_tile_from_tu(Tile_T *tile);
    
};

inline void TileHubTU::receive_tile_from_tu(Tile_T *tile) {
    assert(tile != nullptr);
    assert(tile->check_valid());
    int i = find_position_by_tid(tile->tile_id);
    assert(p_state[i] == TileHub::State::TILING);
    p_state[i] = TileHub::State::READY;
    
    throw runtime_error("need fix");
    if (Config::coalesce) {
        assert(!tile_to_assign.empty());
        if (tile_to_assign.back().empty()) {
            tile_to_assign.back().push_back(i);
            tile_to_assign_elements =
                    tile->forward_edge_total * Config::num_element_feat_edge
                    + tile->source_vertex_total * Config::num_element_feat_src;
            assert(tile_to_assign_elements <= Config::num_total_element_tile);
        } else if (
                (tile->forward_edge_total * Config::num_element_feat_edge
                 + tile->source_vertex_total * Config::num_element_feat_src
                 + tile_to_assign_elements > Config::num_total_element_tile) ||
                (p_data[tile_to_assign.back()[0]]->target_vertex_partition_id
                 != tile->target_vertex_partition_id)) {
            tile_to_assign.push_back({i});
        } else { tile_to_assign.back().push_back(i); }
        
        if (tile->output
            || (num_tile_onchip - num_tiles_ready == 0
                && tile_to_assign.size() < 2)) {
            tile_to_assign.emplace_back();
        }
        
    } else { // disable tile-coalesce
        assert(!tile_to_assign.empty());
        assert(tile_to_assign.back().empty());
        tile_to_assign.back().push_back(i);
        tile_to_assign.emplace_back();
    }
}

inline void TileHubTU::set_position_end() {
    auto i = q_pi_empty.front();
    assert(p_data[i] == nullptr);
    assert(p_state[i] == TileHub::State::READY);
    this->p_state[i] = TileHub::State::END;
    q_pi_empty.pop_front();
}

inline Tile_T *TileHubTU::get_tile_tiling() {
    auto i = q_pi_empty.front();
    assert(p_data[i] == nullptr);
    p_data[i] = new Tile_T();
    assert(p_state[i] == TileHub::State::READY);
    p_state[i] = TileHub::State::TILING;
    q_pi_empty.pop_front();
    return p_data[i];
}

inline bool TileHubTU::check_new_position_tiling() {
    if (q_pi_empty.empty()) return false;
    else return true;
}

#endif //CPPMAIN_TILE_HUB_TU_H
