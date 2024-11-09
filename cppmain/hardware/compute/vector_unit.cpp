//
// Created by Flowerbeach on 2021/2/4.
//

#include <algorithm>
#include "vector_unit.h"
#include "../helps.h"
// #include "../memory/tile_hub/tile_hub.h"

void VectorUnit::_issue(num_op_t &ops, num_access_t &accesses) {
    if (op_target->name == ISA::NONE) { // NONE
        latency_SIMD_NONE(num_cycles_operator, ops, accesses);
    } else if (op_target->name == ISA::GEMV) { // GEMV
        latency_SIMD_GEMV(num_cycles_operator, ops, accesses);
    } else if (count(LIST_OP_ASMD_S.begin(), LIST_OP_ASMD_S.end(),
                     op_target->name) > 0) { // ASMD_V
        latency_SIMD_ASMD_S(num_cycles_operator, ops, accesses);
    } else if (count(LIST_OP_ASMD_V.begin(), LIST_OP_ASMD_V.end(),
                     op_target->name) > 0) { // ASMD_M
        latency_SIMD_ASMD_V(num_cycles_operator, ops, accesses);
    } else if (count(LIST_OP_SCATTER.begin(), LIST_OP_SCATTER.end(),
                     op_target->name) > 0) { // SCATTER
        // Tile_COO *tiles = graph_unit->do_scatter(tile_ids);
        tile_coalesced tiles = th->get_tiles_by_tids(op_target->tile_ids);
        latency_SIMD_scatter(num_cycles_operator, ops, accesses, tiles);
    } else if (count(LIST_OP_GATHER.begin(), LIST_OP_GATHER.end(),
                     op_target->name) > 0) { // GATHER
        tile_coalesced tiles = th->get_tiles_by_tids(op_target->tile_ids);
        latency_SIMD_gather(num_cycles_operator, ops, accesses, tiles);
    } else if (count(LIST_OP_SPECIAL.begin(), LIST_OP_SPECIAL.end(),
                     op_target->name) > 0) { // EXP, RELU, TANH, SIGMOID
        latency_SIMD_SPEC(num_cycles_operator, ops, accesses);
    } else throw runtime_error("VectorUnit::issue_compute");
    assert (op_target->name == ISA::NONE || double(ops) / num_cycles_operator <= num_ops_cycle_total);
    num_cycles_operator += latency_decode;
}

void VectorUnit::latency_SIMD_scatter(long &cycle, num_op_t &ops, num_access_t &accesses,
                                      const tile_coalesced &tiles) {
    cycle = 0;
    long reads = 0, writes = 0;
    
    int M = op_target->operands[0];
    for (auto tile: tiles) {
        if (op_target->name == ISA::SCTR_F) {
            reads += tile->source_vertex_total * ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT);
            writes += tile->forward_edge_total * ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT);
        } else if (op_target->name == ISA::SCTR_B) {
            reads += tile->target_vertex_total * ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT);
            writes += tile->forward_edge_total * ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT);
        } else throw runtime_error("latency_SIMD_scatter.\n");
        cycle += ceil(double(tile->forward_edge_total) / num_block) * ceil(double(M) / num_pe) * factor_special;
    }
    accesses = std::make_tuple(reads, writes);
    ops = 0;
    
}

void VectorUnit::latency_SIMD_gather(long &cycle, num_op_t &ops, num_access_t &accesses,
                                     const tile_coalesced &tiles) {
    assert(count(LIST_OP_GATHER.begin(), LIST_OP_GATHER.end(), op_target->name) > 0);
    int M = op_target->operands[0];
    double latency_per_edge = ceil(double(M) / num_pe) * factor_asmd;
    
    vector<long> latency_block;
    long reads = 0, writes = 0;
    cycle = 0;
    ops = 0;
    
    for (auto tile: tiles) {
        int edge_total, vertex_total;
        if (Config::vu_balance) { latency_block.resize(1, 0); }
        else { latency_block.resize(num_block, 0); }
        
        if (op_target->name == ISA::GTHR_F_SUM
            || op_target->name == ISA::GTHR_F_MAX) {
            edge_total = tile->forward_edge_total;
            vertex_total = tile->target_vertex_total;
            
            vector<long> frequency(vertex_total, 0);
            for (int ei = 0; ei < edge_total; ei++)
                frequency[tile->forward_target_idx[ei] - tile->target_vertex_begin]++;
            for (int i = 0; i < vertex_total; i++) {
                auto block_min_idx = min_element(latency_block.begin(), latency_block.end());
                (*block_min_idx) += ceil(double(frequency[i]) / num_block) * latency_per_edge;
            }// c++ sort the map in ascending order by default.
        } else if (op_target->name == ISA::GTHR_B_SUM
                   || op_target->name == ISA::GTHR_B_MAX) {
            throw runtime_error("GTHR_B_SUM/GTHR_B_MAX not implemented\n");
        } else throw runtime_error("latency_SIMD_gather\n");
        
        reads += edge_total * ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT);
        writes += vertex_total * ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT);
        if (Config::vu_balance) {
            cycle += ceil(*max_element(latency_block.begin(), latency_block.end()) / num_block);
        } else { cycle += *max_element(latency_block.begin(), latency_block.end()); }
        ops += (tile->forward_edge_total - tile->source_vertex_total) * M;
    }
    accesses = std::make_tuple(reads, writes);
}

void VectorUnit::latency_SIMD_GEMV(long &cycle, num_op_t &ops, num_access_t &accesses) {
    assert(op_target->name == ISA::GEMV);
    int M = op_target->operands[0];
    int K = op_target->operands[1];
    
    cycle = ceil(double(M) / num_block) * ceil(double(K) / num_pe) * factor_gemv;
    accesses = std::make_tuple((M + 1) * ceil(double(K) / Config::SIZE_BANK_ROW_ELEMENT), M);
    ops = M * K * factor_gemv;
}

void VectorUnit::latency_SIMD_ASMD_S(long &cycle, num_op_t &ops, num_access_t &accesses) {
    int M = ComputeUnit::op_target->operands[0];
    
    if (op_target->name == ISA::ADD_VS
        || op_target->name == ISA::SUB_VS
        || op_target->name == ISA::MUL_VS
        || op_target->name == ISA::DIV_VS) {
        int N = ComputeUnit::op_target->operands[1];
        
        accesses = std::make_tuple(
                M * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT) + ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT),
                M * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT) + ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT));
        
        cycle = ceil(double(M) / num_block) * ceil(double(N) / num_pe) * factor_asmd;
        ops = M * N * factor_asmd;
        
    } else {
        accesses = std::make_tuple(ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT),
                                   ceil(double(M) / Config::SIZE_BANK_ROW_ELEMENT));
        cycle = ceil(ceil(double(M) / num_pe) / num_block) * factor_asmd;
        ops = M * factor_asmd;
    }
}

void VectorUnit::latency_SIMD_ASMD_V(long &cycle, num_op_t &ops, num_access_t &accesses) {
    int M = op_target->operands[0];
    int N = op_target->operands[1];
    if (op_target->name == ISA::ADD_VV_F
        || op_target->name == ISA::SUB_VV_F
        || op_target->name == ISA::MUL_VV_F
        || op_target->name == ISA::DIV_VV_F) {
        accesses = std::make_tuple(M * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT) * 2,
                                   M * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT));
    } else if (op_target->name == ISA::ADD_VV_W
               || op_target->name == ISA::SUB_VV_W
               || op_target->name == ISA::MUL_VV_W
               || op_target->name == ISA::DIV_VV_W) {
        accesses = std::make_tuple((M + 1) * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT),
                                   M * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT));
    } else throw runtime_error("latency_SIMD_ASMD_V");
    
    cycle = ceil(double(M) / num_block) * ceil(double(N) / num_pe) * factor_asmd;
    ops = M * N * factor_asmd;
}

void VectorUnit::latency_SIMD_SPEC(long &cycle, num_op_t &ops, num_access_t &accesses) {
    int M = op_target->operands[0];
    int N = op_target->operands[1];
    
    cycle = ceil(double(M) / num_block) * ceil(double(N) / num_pe) * factor_special;
    accesses = std::make_tuple(M * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT),
                               M * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT));
    ops = M * N * factor_special;
}

void VectorUnit::latency_SIMD_NONE(long &cycle, num_op_t &ops, num_access_t &accesses) {
    accesses = std::make_tuple(0, 0);
    cycle = 0;
    ops = 0;
}