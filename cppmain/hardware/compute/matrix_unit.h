//
// Created by SmartCabbage on 2021/1/29.
//

#ifndef CPPMAIN_MATRIX_UNIT_H
#define CPPMAIN_MATRIX_UNIT_H

#include "compute.h"
#include <tuple>

class MatrixUnit : public ComputeUnit {
public:
    explicit MatrixUnit(TileHub *th, BufferGroup *bg)
            : ComputeUnit(EDevice::MU, th, bg) {
        num_ops_cycle_total = sa_nrow * sa_ncol * 2;
    }

protected:
    const int sa_nrow = 32;
    const int sa_ncol = 128;
    const int cycle_per_switch = sa_nrow;//save rst to rst buffer
    const int cycle_per_move_compute = 1;
    
    void _issue(num_op_t &ops, num_access_t &accesses) override;
    void latency_SA_GEMM_output(long &cycle, num_op_t &ops, num_access_t &accesses);
    void latency_SA_BMM_output(long &cycle, num_op_t &ops, num_access_t &accesses, const tile_coalesced &tiles);
};

#endif //CPPMAIN_MATRIX_UNIT_H
