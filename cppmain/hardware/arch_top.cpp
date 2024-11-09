//
// Created by SmartCabbage on 2021/2/2.
//

#include "arch_top.h"

ArchTop::ArchTop() {
    if (Config::name_arch == EArch::base) {
        
        mah = new MemoryAccessHandler();
        th = new TileHub(mah);
        mah->set_th(th);
        
        vu_group.resize(Config::num_vu);
        mu_group.resize(Config::num_mu);
        for (int i = 0; i < Config::num_vu; i++)
            vu_group[i] = new VectorUnit(th, mah->get_bg());
        for (int i = 0; i < Config::num_mu; i++)
            mu_group[i] = new MatrixUnit(th, mah->get_bg());
        
        ctrl = new control::CtrlTop(mah, th, vu_group, mu_group);
        
    } else {
        th = new TileHubTU();
        mah = new MemoryAccessHandler();
        if (Config::name_tu == ETU::csr)
            throw runtime_error("tu-csr need refactor\n");
            // tu = new tiling::TilingUnitCSR(mah, (TileHubTU *) th);
        else if (Config::name_tu == ETU::csc)
            throw runtime_error("tu-csc need refactor\n");
            // tu = new tiling::TilingUnitCSC(mah, (TileHubTU *) th);
        else if (Config::name_tu == ETU::bitmap)
            tu = new tiling::TilingUnitBitmap(mah, (TileHubTU *) th);
        else throw runtime_error("name_arch");
        mah->set_th(th);
        
        vu_group.resize(Config::num_vu);
        mu_group.resize(Config::num_mu);
        for (int i = 0; i < Config::num_vu; i++)
            vu_group[i] = new VectorUnit(th, mah->get_bg());
        for (int i = 0; i < Config::num_mu; i++)
            mu_group[i] = new MatrixUnit(th, mah->get_bg());
        
        ctrl = new control::CtrlTop(mah, th, vu_group, mu_group, tu);
    }
}

ArchTop::~ArchTop() {
    delete mah;
    for (auto *vu: vu_group) delete vu;
    for (auto *mu: mu_group) delete mu;
    delete ctrl;
    delete th;
    delete tu;
}

void ArchTop::tick() {
#ifdef _DEBUG_EXE
    cout << "===============================\n";
    cout << string("ArchTop: cycle ") + to_string(cycle_curr) + "\n";
#endif
    check_end();
    
    mah->tick();
    for (auto &vu: vu_group) vu->tick();
    for (auto &mu: mu_group) mu->tick();
    ctrl->tick();
    th->tick();
    
    if (tu) tu->tick();
    
    cycle_curr++;
    if (cycle_curr == 1000000) flag_end = true;
}

bool ArchTop::check_flag_end() const { return flag_end; }

long long ArchTop::get_num_cycles() const { return cycle_curr; }

long ArchTop::get_volume_dram_read() const { return mah->get_volume_data_read(); }

long ArchTop::get_volume_dram_write() const { return mah->get_volume_data_write(); }

double ArchTop::get_utilization_mu() const {
    double utilization = 0;
    for (auto i: mu_group) {
        auto util = i->get_utilization(cycle_curr);
        printf("    single MU: %.2lf %%\n", 100 * util);
        utilization += util;
    }
    return utilization / double(mu_group.size());
}

double ArchTop::get_utilization_vu() const {
    double utilization = 0;
    for (auto i: vu_group) {
        auto util = i->get_utilization(cycle_curr);
        printf("    single VU: %.2lf %%\n", 100 * util);
        utilization += util;
    }
    return utilization / double(vu_group.size());
}

double ArchTop::get_utilization_tu() const {
    if (Config::name_arch == EArch::base) return 0;
    else return tu->get_utilization(cycle_curr);
}

double ArchTop::get_energy_mu_mJ() const {
    double energy = 0;
    for (auto i: mu_group)
        energy += i->get_utilization(cycle_curr)
                  * (double(cycle_curr) / Config::FrequencyArch_MHz / 1024 / 1024)
                  * Config::POWER_MU_mW;
    return energy;
}

double ArchTop::get_energy_vu_mJ() const {
    double energy = 0;
    for (auto i: vu_group)
        energy += i->get_utilization(cycle_curr)
                  * (double(cycle_curr) / Config::FrequencyArch_MHz / 1024 / 1024)
                  * Config::POWER_VU_mW;
    return energy;
}

double ArchTop::get_energy_tu_mJ() const { return tu->get_energy_mJ(); }

double ArchTop::get_energy_buf_mJ() const {
    double dynamic_EB_mJ = double(mah->get_bg()->get_num_accesses_total())
                           / (double(1000) * 1000)
                           * Config::ENERGY_DYNAMIC_EB_nJ;
    double leakage_mJ = double(cycle_curr)
                        / double(Config::FrequencyArch_MHz * 1024 * 1024)
                        * Config::POWER_LEAKAGE_RAM_mW;
    return leakage_mJ + dynamic_EB_mJ;
}

double ArchTop::get_energy_ctrl_mJ() const {
    return double(cycle_curr)
           / double(Config::FrequencyArch_MHz * 1024 * 1024)
           * Config::POWER_CTRL_mW;
}

double ArchTop::get_energy_dram_mJ() const {
    return double(get_volume_dram_read() + get_volume_dram_write())
           / double(double(1000) * 1000 * 1000 / 8)
           * Config::HBM_energy_pJ_bit;
}

void ArchTop::check_end() {
    for (auto vu: vu_group) if (!vu->check_state_ready()) return;
    for (auto mu: mu_group) if (!mu->check_state_ready()) return;
    if (!mah->check_end()) return;
    if (!th->check_state_all_end()) return;
    if (tu && !tu->check_state_end()) { return; }
    flag_end = true;
    
    if (tu) tu->final_assert();
}
