//
// Created by SmartCabbage on 2021/1/29.
//

#ifndef CPPMAIN_HELPS_H
#define CPPMAIN_HELPS_H

#include "../configs/config.h"
#include "arch_top.h"

namespace helps {
    void print_statistics(ArchTop *arch);
    void print_state_periodically(ArchTop *arch, long start, long interval, bool do_print_state);
}

#endif //CPPMAIN_HELPS_H
