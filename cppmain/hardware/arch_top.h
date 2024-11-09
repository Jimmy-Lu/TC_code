//
// Created by SmartCabbage on 2021/1/27.
//

#ifndef CPPMAIN_ARCH_TOP_H
#define CPPMAIN_ARCH_TOP_H

#include "memory/mem_top.h"
#include "compute/vector_unit.h"
#include "compute/matrix_unit.h"
#include "control/ctrl_top.h"
#include "tiling/online/tile_hub_tu.h"
// #include "tiling/online/tiling_csr.h"
// #include "tiling/online/tiling_csc.h"
#include "tiling/online/tiling_bitmap.h"

class ArchTop {
public:
    explicit ArchTop();
    ~ArchTop();
    void tick();
    bool check_flag_end() const;
    inline long get_num_tiles_processed() const { return ctrl->get_tiles_processed(); }
    inline long get_num_threads_launched() const { return ctrl->get_threads_launched(); }
    long get_volume_dram_read() const;
    long get_volume_dram_write() const;
    long long get_num_cycles() const;
    
    double get_utilization_mu() const;
    double get_utilization_vu() const;
    double get_utilization_tu() const;
    
    double get_energy_mu_mJ() const;  // mJ
    double get_energy_vu_mJ() const;  // mJ
    double get_energy_tu_mJ() const;  // mJ
    double get_energy_buf_mJ() const;  // mJ
    double get_energy_dram_mJ() const;  // mJ
    double get_energy_ctrl_mJ() const;  // mJ

public:
    control::CtrlTop *ctrl = nullptr;
    vector<ComputeUnit *> vu_group;
    vector<ComputeUnit *> mu_group;
    MemoryAccessHandler *mah = nullptr;
    tiling::TilingUnit *tu = nullptr;
    TileHub *th = nullptr;

private:
    bool flag_end = false;
    long long cycle_curr = 0;
    
    void check_end();
};

#endif //CPPMAIN_ARCH_TOP_H
