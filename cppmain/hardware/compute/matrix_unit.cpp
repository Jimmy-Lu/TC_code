//
// Created by Flowerbeach on 2021/2/4.
//

#include "matrix_unit.h"
#include "../helps.h"
// #include "../memory/tile_hub/tile_hub.h"

void MatrixUnit::_issue(num_op_t &ops, num_access_t &accesses) {
    if (op_target->name == ISA::GEMM) {
        latency_SA_GEMM_output(num_cycles_operator, ops, accesses);
    } else if (op_target->name == ISA::BMM) {
        tile_coalesced tiles = th->get_tiles_by_tids(op_target->tile_ids);
        latency_SA_BMM_output(num_cycles_operator, ops, accesses, tiles);
    } else throw runtime_error("MatrixUnit::issue_compute");
    assert (double(ops) / num_cycles_operator <= num_ops_cycle_total);
}

void GEMM_output(int M, int N, int K, int sa_nrow, int sa_ncol,
                 int &num_move_compute, int &num_switch);

void MatrixUnit::latency_SA_GEMM_output(long &cycle, num_op_t &ops, num_access_t &accesses) {
    int M = op_target->operands[0];
    int N = op_target->operands[1];
    int K = op_target->operands[2];
    
    int num_switch, num_move_compute;
    GEMM_output(M, N, K, sa_nrow, sa_ncol, num_move_compute, num_switch);
    
    cycle = num_move_compute * cycle_per_move_compute;
    accesses = std::make_tuple(
            M * ceil(double(K) / Config::SIZE_BANK_ROW_ELEMENT)
            + N * ceil(double(K) / Config::SIZE_BANK_ROW_ELEMENT),
            M * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT));
    ops = M * K * N * 2;
}

void MatrixUnit::latency_SA_BMM_output(
        long &cycle, num_op_t &ops, num_access_t &accesses,
        const tile_coalesced &tiles) {
    int M = op_target->operands[0];
    int N = op_target->operands[1];
    int K = op_target->operands[2];
    
    int num_etype = Config::num_edge_types, M_curr, M_cnt = 0;
    long write_cnt = 0, read_cnt = 0;
    int num_switch, num_move_compute;
    
    ops = 0;
    cycle = 0;
    for (auto tile: tiles) {
        for (int etype_curr = 0; etype_curr < num_etype; etype_curr++) {
            M_curr = 0;
            for (int i = 0; i < tile->forward_edge_total; i++)
                if (tile->forward_edge_type[i] == etype_curr) M_curr++;
            
            GEMM_output(M_curr, N, K, sa_nrow, sa_ncol, num_move_compute, num_switch);
            
            ops += M_curr * N * K * 2;
            cycle += num_move_compute * cycle_per_move_compute;
            read_cnt += M_curr * ceil(double(K) / Config::SIZE_BANK_ROW_ELEMENT)
                        + num_switch * N * ceil(double(K) / Config::SIZE_BANK_ROW_ELEMENT);
            write_cnt += M_curr * ceil(double(N) / Config::SIZE_BANK_ROW_ELEMENT);
            M_cnt += M_curr;
        }
    }
    accesses = std::make_tuple(read_cnt, write_cnt);
    assert(M_cnt == M && "MatrixUnit::latency_SA_BMM_output");
}

void GEMM_output(int M, int N, int K, int sa_nrow, int sa_ncol,
                 int &num_move_compute, int &num_switch) {
    int pn, pm;
    pm = ceil(double(M) / sa_nrow);
    pn = ceil(double(N) / sa_ncol);
    num_move_compute = M % sa_nrow + pn * pm * K + N % sa_ncol;
    num_switch = pn * pm;
}