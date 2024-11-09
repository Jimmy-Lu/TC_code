//
// Created by SmartCabbage on 2021/4/7.
//

#ifndef CPPMAIN_MAIN_PARTITION_H
#define CPPMAIN_MAIN_PARTITION_H

#include <unistd.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>

#include "preprocessing/tiling/sparse.h"

// #define TIMING_

void thread_main(GraphLoader *tiling) {
    vector<vid_t> empty_rows_thread(Config::num_workers, 0);
    vector<vid_t> num_tiles_thread(Config::num_workers, 0);
    vector<double> buffer_util_thread(Config::num_workers, 0);
    cout << "Worker launched: ";

#ifdef TIMING_
    clock_t tile_begin, tile_end;
    tile_begin = clock();
#endif
#ifdef OPENMP
#pragma omp parallel num_threads(Config::num_workers)
#endif
    {
#ifdef OPENMP
        int t = omp_get_thread_num();
#else
        int t = 0;
#endif
        cout << to_string(t) + " ";

#ifdef TIMING_
        clock_t tile_begin_l, tile_end_l;
        tile_begin_l = clock();
#endif
        tiling->start_partition(t);
        empty_rows_thread[t] = tiling->empty_rows;
        num_tiles_thread[t] = tiling->num_tiles;
        buffer_util_thread[t] = tiling->buff_util_avg;
#ifdef TIMING_
        tile_end_l = clock();
        double dur = (double) (tile_end_l - tile_begin_l) / CLOCKS_PER_SEC;
        cout << string("\n  process time: ") + to_string(dur) + " sec.";
#endif
    
    }
#ifdef TIMING_
    tile_end = clock();
#endif
    cout << endl;
    double util_avg = 0;
    eid_t empty_rows = 0, num_tiles = 0;
    for (int i = 0; i < Config::num_workers; i++) {
        empty_rows += empty_rows_thread[i];
        num_tiles += num_tiles_thread[i];
        cout << " " << num_tiles_thread[i] << " " << buffer_util_thread[i] << endl;
        util_avg += num_tiles_thread[i] * buffer_util_thread[i];
    }
    cout << "Number of Tiles: " << num_tiles << endl;
    cout << "Number of EmptyRows: " << empty_rows << endl;
    cout << "Buffer Utilization: " << util_avg / double(num_tiles) * 100 << " %" << endl;
#ifdef TIMING_
    auto dur = (double) (tile_end - tile_begin) / CLOCKS_PER_SEC;
    cout << string("Tiling Time: ") + to_string(dur) + " sec.\n";
#endif
}

void thread_trash(const fs::path &p) { fs::remove_all(p); }

void start_run() {
#ifdef TIMING_
    clock_t load_begin, load_end;
    load_begin = clock();
#endif
    auto *tiling = new GraphLoader();
#ifdef TIMING_
    load_end = clock();
    double loading_time = (double) (load_end - load_begin) / CLOCKS_PER_SEC;
    printf("Loading Time:    %lf sec.\n", loading_time);
#endif
    
    thread t_trash = thread(thread_trash, Config::path_trash);
    thread t_main = thread(thread_main, tiling);
    t_trash.join();
    t_main.join();
    delete tiling;
}

#endif //CPPMAIN_MAIN_PARTITION_H
