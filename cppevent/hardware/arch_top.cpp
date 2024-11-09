//
// Created by SmartCabbage on 2021/2/2.
//

#include "arch_top.h"

ArchTop::ArchTop() {
    
    mah = new MemoryAccessHandler();
    sh = new ShardHub(mah);
    mah->lsu->set_sh(sh);
    
    vu_group.resize(Config::num_vu);
    mu_group.resize(Config::num_mu);
    for (int i = 0; i < Config::num_vu; i++)
        vu_group[i] = new VectorUnit(sh, mah->bg);
    for (int i = 0; i < Config::num_mu; i++)
        mu_group[i] = new MatrixUnit(sh, mah->bg);
    ctrl = new control::CtrlTop(mah, sh, vu_group, mu_group);
    for (int i = 0; i < Config::num_vu; i++)
        vu_group[i]->set_ctrl(ctrl);
    for (int i = 0; i < Config::num_mu; i++)
        mu_group[i]->set_ctrl(ctrl);
    mah->set_ctrl(ctrl);
    
}
ArchTop::~ArchTop() {
    delete mah;
    for (auto *vu: vu_group) delete vu;
    for (auto *mu: mu_group) delete mu;
    delete ctrl;
    delete sh;
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
    sh->tick();
    
    cycle_curr++;
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

double ArchTop::get_energy_buf_mJ() const {
    double dynamic_EB_mJ = double(mah->bg->get_num_accesses_total())
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
    if (ctrl->ctrl_state != control::CtrlTop::CtrlState::end) return;
    if (!sh->check_state_all_end()) return;
    if (!mah->check_end()) return;
    
    for (auto vu: vu_group) if (!vu->check_state_ready()) return;
    for (auto mu: mu_group) if (!mu->check_state_ready()) return;
    
    flag_end = true;
    
    mah->finish_ramulator();
}
