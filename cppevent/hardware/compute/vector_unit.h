//
// Created by SmartCabbage on 2021/1/29.
//

#ifndef CPPMAIN_VECTOR_UNIT_H
#define CPPMAIN_VECTOR_UNIT_H

#include "compute.h"

class VectorUnit : public ComputeUnit {
public:
    explicit VectorUnit(ShardHub *sh, BufferGroup *bg)
            : ComputeUnit(EDevice::VU, sh, bg) {
        num_ops_cycle_total = num_pe * num_block;
    }

protected:
    const int num_pe = 32;
    const int num_block = 16;
    const double factor_asmd = 1;
    const double factor_gemv = 1;
    const double factor_special = 1;
    const int latency_decode = 1;
    
    void _issue(num_op_t &ops, num_access_t &accesses) override;
    void latency_SIMD_scatter(long &cycle, num_op_t &ops, num_access_t &accesses, const shard_coalesced &shards);
    void latency_SIMD_gather(long &cycle, num_op_t &ops, num_access_t &accesses, const shard_coalesced &shards);
    void latency_SIMD_GEMV(long &cycle, num_op_t &ops, num_access_t &accesses);
    void latency_SIMD_ASMD_S(long &cycle, num_op_t &ops, num_access_t &accesses);
    void latency_SIMD_ASMD_V(long &cycle, num_op_t &ops, num_access_t &accesses);
    void latency_SIMD_SPEC(long &cycle, num_op_t &ops, num_access_t &accesses);
    static void latency_SIMD_NONE(long &cycle, num_op_t &ops, num_access_t &accesses);
    
    Op *fetch_op_from_ctrl() override;
};

#endif //CPPMAIN_VECTOR_UNIT_H
