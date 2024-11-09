//
// Created by SmartCabbage on 2021/4/7.
//

#ifndef CPPMAIN_MAIN_PARTITIONING_H
#define CPPMAIN_MAIN_PARTITIONING_H

#include <unistd.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>

#include "preprocessing/partitioning/sparse.h"

// #define TIMING_

void thread_main(GraphLoader *partitioning) {
    vector<vid_t> empty_rows_thread(Config::num_workers, 0);
    vector<vid_t> num_shards_thread(Config::num_workers, 0);
    vector<double> buffer_util_thread(Config::num_workers, 0);
    cout << "Worker launched: ";

#ifdef TIMING_
    clock_t shard_begin, shard_end;
    shard_begin = clock();
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
        clock_t shard_begin_l, shard_end_l;
        shard_begin_l = clock();
#endif
        partitioning->start_partitioning(t);
        empty_rows_thread[t] = partitioning->empty_rows;
        num_shards_thread[t] = partitioning->num_shards;
        buffer_util_thread[t] = partitioning->buff_util_avg;
#ifdef TIMING_
        shard_end_l = clock();
        double dur = (double) (shard_end_l - shard_begin_l) / CLOCKS_PER_SEC;
        cout << string("\n  process time: ") + to_string(dur) + " sec.";
#endif
    
    }
#ifdef TIMING_
    shard_end = clock();
#endif
    cout << endl;
    double util_avg = 0;
    eid_t empty_rows = 0, num_shards = 0;
    for (int i = 0; i < Config::num_workers; i++) {
        empty_rows += empty_rows_thread[i];
        num_shards += num_shards_thread[i];
        cout << " " << num_shards_thread[i] << " " << buffer_util_thread[i] << endl;
        util_avg += num_shards_thread[i] * buffer_util_thread[i];
    }
    cout << "Number of Shards: " << num_shards << endl;
    cout << "Number of EmptyRows: " << empty_rows << endl;
    cout << "Buffer Utilization: " << util_avg / double(num_shards) * 100 << " %" << endl;
#ifdef TIMING_
    auto dur = (double) (shard_end - shard_begin) / CLOCKS_PER_SEC;
    cout << string("Partitioning Time: ") + to_string(dur) + " sec.\n";
#endif
}

void thread_trash(const fs::path &p) { fs::remove_all(p); }

void start_run() {
#ifdef TIMING_
    clock_t load_begin, load_end;
    load_begin = clock();
#endif
    auto *partitioning = new GraphLoader();
#ifdef TIMING_
    load_end = clock();
    double loading_time = (double) (load_end - load_begin) / CLOCKS_PER_SEC;
    printf("Loading Time:    %lf sec.\n", loading_time);
#endif
    
    thread t_trash = thread(thread_trash, Config::path_trash);
    thread t_main = thread(thread_main, partitioning);
    t_trash.join();
    t_main.join();
    delete partitioning;
}

#endif //CPPMAIN_MAIN_PARTITIONING_H
