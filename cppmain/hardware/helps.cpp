//
// Created by Flowerbeach on 2021/3/23.
//
#include "helps.h"

void helps::print_statistics(ArchTop *arch) {
    Config::print_config();
    printf("--------- statistics -----------\n");
    double num_cycles_ms = double(arch->get_num_cycles()) / (1024 * 1024 * Config::FrequencyArch_MHz) * 1000;
    long num_tiles = arch->get_num_tiles_processed();
    long coalesced = arch->get_num_threads_launched();
    printf("  Cycle#: %.3lf ms, Tile#: %ld, Coalesced: %ld\n", num_cycles_ms, num_tiles, coalesced);
    // if (num_tiles != arch->mah->get_num_tiles()) throw runtime_error("tile number.");
    
    double read_MB = double(arch->get_volume_dram_read()) / 1024 / 1024;
    double write_MB = double(arch->get_volume_dram_write()) / 1024 / 1024;
    printf("    ----------------\n");
    printf("  Off-chip read:    %.3lf MB\n", read_MB);
    printf("  Off-chip write:   %.3lf MB\n", write_MB);
    double bandwidth = double(write_MB + read_MB) / num_cycles_ms;
    printf("  Bandwidth avg:    %.1f GB/s,   %.1f %%\n", bandwidth, bandwidth * 100 / Config::BandwidthDRAM_GB_s);
    printf("    ----------------\n");
    printf("  MU utilization:   %.2lf %%\n", 100 * arch->get_utilization_mu());
    printf("  VU utilization:   %.2lf %%\n", 100 * arch->get_utilization_vu());
    printf("  TU utilization:   %.2lf %%\n", 100 * arch->get_utilization_tu());
    printf("    ----------------\n");
    
    //energy
    double energy_mu_mJ = arch->get_energy_mu_mJ();
    double energy_vu_mJ = arch->get_energy_vu_mJ();
    double energy_buf_mJ = arch->get_energy_buf_mJ();
    double energy_ctrl_mJ = arch->get_energy_ctrl_mJ();
    double energy_dram_mJ = arch->get_energy_dram_mJ();
    double energy_tu_mJ = (Config::name_arch == EArch::base) ? 0 : arch->get_energy_tu_mJ();
    double energy_total_mJ = energy_dram_mJ + energy_ctrl_mJ + energy_buf_mJ
                             + energy_tu_mJ + energy_vu_mJ + energy_mu_mJ;
    printf("    ----------------\n");
    printf("  Energy MU   :   %.4lf mJ\n", energy_mu_mJ);
    printf("  Energy VU   :   %.4lf mJ\n", energy_vu_mJ);
    printf("  Energy TU   :   %.4lf mJ\n", energy_tu_mJ);
    printf("  Energy Buf  :   %.4lf mJ\n", energy_buf_mJ);
    printf("  Energy CTRL :   %.4lf mJ\n", energy_ctrl_mJ);
    printf("  Energy DRAM :   %.4lf mJ\n", energy_dram_mJ);
    printf("  Energy Total:   %.4lf mJ\n", energy_total_mJ);
    printf("---------------------------------\n");
    
    // ramulator statistics
    arch->mah->mem->finish();
    Stats::statlist.printall();
}

void helps::print_state_periodically(ArchTop *arch, long start, long interval, bool do_print_state) {
    long cycle = arch->get_num_cycles();
    if (cycle < start) return;
    if (arch->get_num_cycles() % interval == 0) {
        cout << cycle << ' ';
        if (cycle % (interval * 5) == 0) {
            cout << endl;
        }
    }
}