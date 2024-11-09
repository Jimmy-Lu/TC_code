//
// Created by flowe on 2021/7/5.
//

#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include <parallel/algorithm>

#include "reordering.h"
#include "sliding_queue.h"

vector<vid_t> *generateHubClusterDBGMapping(const vid_t nv, const eid_t ne,
                                            const vector<vid_t> &in_degrees,
                                            const vector<vid_t> &out_degrees) {
    auto *mapping_use = new vector<vid_t>(nv);
    
    uint32_t avg_vertex = ne / nv;
    
    const int num_buckets = 2;
    avg_vertex = avg_vertex;
    uint32_t bucket_threshold[] = {avg_vertex, static_cast<uint32_t>(-1)};
    
    // vector<uint32_t> bucket_vertices[num_buckets];
    const int num_threads = getWorkers();
    vector<uint32_t> local_buckets[num_threads][num_buckets];
    
    {
#pragma omp parallel for schedule(static)
        for (uint64_t i = 0; i < nv; i++) {
            for (unsigned int j = 0; j < num_buckets; j++) {
                const uintE &count = in_degrees[i];
                if (count <= bucket_threshold[j]) {
                    local_buckets[omp_get_thread_num()][j].push_back(i);
                    break;
                }
            }
        }
    }
    
    int temp_k = 0;
    uint32_t start_k[num_threads][num_buckets];
    for (int32_t j = num_buckets - 1; j >= 0; j--) {
        for (int t = 0; t < num_threads; t++) {
            start_k[t][j] = temp_k;
            temp_k += local_buckets[t][j].size();
        }
    }

#pragma omp parallel for schedule(static)
    for (int t = 0; t < num_threads; t++) {
        for (int32_t j = num_buckets - 1; j >= 0; j--) {
            const vector<uint32_t> &current_bucket = local_buckets[t][j];
            int k = start_k[t][j];
            const size_t &size = current_bucket.size();
            for (uint32_t i = 0; i < size; i++) {
                (*mapping_use)[current_bucket[i]] = k++;
            }
        }
    }
    
    for (uint64_t i = 0; i < num_threads; i++) {
        for (unsigned int j = 0; j < num_buckets; j++) {
            local_buckets[i][j].clear();
        }
    }
    return mapping_use;
}

vector<vid_t> *generateDBGMapping(const vid_t nv, const eid_t ne,
                                  const vector<vid_t> &in_degrees,
                                  const vector<vid_t> &out_degrees) {
    auto *mapping_use = new vector<vid_t>(nv);
    
    uint32_t avg_vertex = ne / nv;
    const uint32_t &av = avg_vertex;
    
    uint32_t bucket_threshold[] = {av / 2, av, av * 2, av * 4, av * 8, av * 16, av * 32, av * 64, av * 128, av * 256,
                                   av * 512, static_cast<uint32_t>(-1)};
    int num_buckets = 8;
    if (num_buckets > 11) {
        // if you really want to increase the bucket count, add more thresholds to the bucket_threshold above.
        cout << "Unsupported bucket size: " << num_buckets << std::endl;
        assert(0);
    }
    bucket_threshold[num_buckets - 1] = static_cast<uint32_t>(-1);
    
    // vector<uint32_t> bucket_vertices[num_buckets];
    const int num_threads = getWorkers();
    vector<uint32_t> local_buckets[num_threads][num_buckets];
    
    {
#pragma omp parallel for schedule(static)
        for (uint64_t i = 0; i < nv; i++) {
            for (unsigned int j = 0; j < num_buckets; j++) {
                const uintE &count = in_degrees[i];
                if (count <= bucket_threshold[j]) {
                    local_buckets[omp_get_thread_num()][j].push_back(i);
                    break;
                }
            }
        }
    }
    
    int temp_k = 0;
    uint32_t start_k[num_threads][num_buckets];
    for (int32_t j = num_buckets - 1; j >= 0; j--) {
        for (int t = 0; t < num_threads; t++) {
            start_k[t][j] = temp_k;
            temp_k += local_buckets[t][j].size();
        }
    }

#pragma omp parallel for schedule(static)
    for (int t = 0; t < num_threads; t++) {
        for (int32_t j = num_buckets - 1; j >= 0; j--) {
            const vector<uint32_t> &current_bucket = local_buckets[t][j];
            int k = start_k[t][j];
            const size_t &size = current_bucket.size();
            for (uint32_t i = 0; i < size; i++) {
                (*mapping_use)[current_bucket[i]] = k++;
            }
        }
    }
    
    for (uint64_t i = 0; i < num_threads; i++) {
        for (unsigned int j = 0; j < num_buckets; j++) {
            local_buckets[i][j].clear();
        }
    }
    return mapping_use;
}

vector<vid_t> *generateSortMapping(const vid_t nv, const eid_t ne,
                                   const vector<vid_t> &in_degrees,
                                   const vector<vid_t> &out_degrees) {
    auto *mapping_use = new vector<vid_t>(nv);
    
    vector<degree_nodeid_t> degree_id_pairs(nv);
    
    {
#pragma omp parallel for
        for (uintE v = 0; v < nv; ++v) {
            degree_id_pairs[v] = std::make_pair(in_degrees[v], v);
        }
    }
    
    __gnu_parallel::sort(degree_id_pairs.begin(), degree_id_pairs.end(),
                         std::greater<degree_nodeid_t>());

#pragma omp parallel for
    for (uintE i = 0; i < nv; ++i) {
        (*mapping_use)[degree_id_pairs[i].second] = i;
    }
    
    return mapping_use;
}

vector<vid_t> *generateHubClusterMapping(const vid_t nv, const eid_t ne,
                                         const vector<vid_t> &in_degrees,
                                         const vector<vid_t> &out_degrees) {
    auto *mapping_use = new vector<vid_t>(nv);
    
    
    
    // vector<degree_nodeid_t> degree_id_pairs(nv);
    uintT avgDegree = ne / nv;
    // uintT hubCount{0};
    
    const int PADDING = 64 / sizeof(uintE);
    auto *localOffsets = new uintE[getWorkers() * PADDING]();
    uintE partitionSz = nv / getWorkers();

#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        uintE startID = partitionSz * tid;
        uintE stopID = partitionSz * (tid + 1);
        if (tid == getWorkers() - 1) {
            stopID = nv;
        }
        for (uintE n = startID; n < stopID; ++n) {
            if (in_degrees[n] > avgDegree) {
                ++localOffsets[tid * PADDING];
                (*mapping_use)[n] = 1;
            }
        }
    }
    uintE sum{0};
    for (int tid = 0; tid < getWorkers(); ++tid) {
        auto origCount = localOffsets[tid * PADDING];
        localOffsets[tid * PADDING] = sum;
        sum += origCount;
    }
    
    /* Step II - assign a remap for the hub vertices first */
#pragma omp parallel
    {
        uintE localCtr{0};
        int tid = omp_get_thread_num();
        uintE startID = partitionSz * tid;
        uintE stopID = partitionSz * (tid + 1);
        if (tid == getWorkers() - 1) {
            stopID = nv;
        }
        for (uintE n = startID; n < stopID; ++n) {
            if ((*mapping_use)[n] != UINT_E_MAX) {
                (*mapping_use)[n] = localOffsets[tid * PADDING] + localCtr;
                ++localCtr;
            }
        }
    }
    delete[] localOffsets;
    
    /* Step III - assigning a remap for (easy) non hub vertices */
    auto numHubs = sum;
    SlidingQueue<uintE> queue(numHubs);
#pragma omp parallel
    {
        //assert(getWorkers() == 56);
        QueueBuffer<uintE> lqueue(queue, numHubs / getWorkers());
#pragma omp for
        for (uintE n = numHubs; n < nv; ++n) {
            if ((*mapping_use)[n] == UINT_E_MAX) {
                // This steps preserves the ordering of the original graph (as much as possible)
                (*mapping_use)[n] = n;
            } else {
                uintE remappedTo = (*mapping_use)[n];
                if ((*mapping_use)[remappedTo] == UINT_E_MAX) {
                    // safe to swap Ids because the original vertex is a non-hub
                    (*mapping_use)[remappedTo] = n;
                } else {
                    // Cannot swap ids because original vertex was a hub (swapping
                    // would disturb sorted ordering of hubs - not allowed)
                    lqueue.push_back(n);
                }
            }
        }
        lqueue.flush();
    }
    queue.slide_window(); //the queue keeps a list of vertices where a simple swap of locations is not possible
    
    /* Step IV - assigning remaps for remaining non hubs */
    uintE unassignedCtr{0};
    auto q_iter = queue.begin();
#pragma omp parallel for
    for (uintE n = 0; n < numHubs; ++n) {
        if ((*mapping_use)[n] == UINT_E_MAX) {
            uintE u = *(q_iter + __sync_fetch_and_add(&unassignedCtr, 1));
            (*mapping_use)[n] = u;
        }
    }
    
    return mapping_use;
}

vector<vid_t> *generateHubSortMapping(const vid_t nv, const eid_t ne,
                                      const vector<vid_t> &in_degrees,
                                      const vector<vid_t> &out_degrees) {
    auto *mapping_use = new vector<vid_t>(nv);
    
    vector<degree_nodeid_t> degree_id_pairs(nv);
    uintT avgDegree = ne / nv;
    uintT hubCount{0};
    
    /* STEP I - collect degrees of all vertices */
#pragma omp parallel for reduction(+ : hubCount)
    for (uintE v = 0; v < nv; ++v) {
        degree_id_pairs[v] = std::make_pair(in_degrees[v], v);
        if (in_degrees[v] > avgDegree) {
            ++hubCount;
        }
    }
    
    /* Step II - sort the degrees in parallel */
    __gnu_parallel::sort(degree_id_pairs.begin(), degree_id_pairs.end(),
                         std::greater<degree_nodeid_t>());
    
    /* Step III - make a remap based on the sorted degree list [Only for hubs] */
#pragma omp parallel for
    for (uintE n = 0; n < hubCount; ++n) {
        (*mapping_use)[degree_id_pairs[n].second] = n;
    }
    //clearing space from degree pairs
    vector<degree_nodeid_t>().swap(degree_id_pairs);
    
    /* Step IV - assigning a remap for (easy) non hub vertices */
    auto numHubs = hubCount;
    SlidingQueue<uintE> queue(numHubs);
#pragma omp parallel
    {
        QueueBuffer<uintE> lqueue(queue, numHubs / getWorkers());
#pragma omp for
        for (uintE n = numHubs; n < nv; ++n) {
            if ((*mapping_use)[n] == UINT_E_MAX) {
                // This steps preserves the ordering of the original graph (as much as possible)
                (*mapping_use)[n] = n;
            } else {
                uintE remappedTo = (*mapping_use)[n];
                if ((*mapping_use)[remappedTo] == UINT_E_MAX) {
                    // safe to swap Ids because the original vertex is a non-hub
                    (*mapping_use)[remappedTo] = n;
                } else {
                    // Cannot swap ids because original vertex was a hub (swapping
                    // would disturb sorted ordering of hubs - not allowed)
                    lqueue.push_back(n);
                }
            }
        }
        lqueue.flush();
    }
    queue.slide_window(); //the queue keeps a list of vertices where a simple swap of locations is not possible
    /* Step V - assigning remaps for remaining non hubs */
    uintE unassignedCtr{0};
    auto q_iter = queue.begin();
#pragma omp parallel for
    for (uintE n = 0; n < numHubs; ++n) {
        if ((*mapping_use)[n] == UINT_E_MAX) {
            uintE u = *(q_iter + __sync_fetch_and_add(&unassignedCtr, 1));
            (*mapping_use)[n] = u;
        }
    }
    
    return mapping_use;
}

vector<vid_t> *generateHubSortDBGMapping(const vid_t nv, const eid_t ne,
                                         const vector<vid_t> &in_degrees,
                                         const vector<vid_t> &out_degrees) {
    auto *mapping_use = new vector<vid_t>(nv);
    
    uintT avgDegree = ne / nv;
    uintT hubCount{0};
    
    const int num_threads = getWorkers();
    vector<degree_nodeid_t> local_degree_id_pairs[num_threads];
    uintE slice = nv / num_threads;
    uintE start[num_threads];
    uintE end[num_threads];
    uintE hub_count[num_threads];
    uintE non_hub_count[num_threads];
    uintE new_index[num_threads];
    for (int t = 0; t < num_threads; t++) {
        start[t] = t * slice;
        end[t] = (t + 1) * slice;
        hub_count[t] = 0;
    }
    end[num_threads - 1] = nv;

#pragma omp parallel for schedule(static) num_threads(num_threads)
    for (uintE t = 0; t < num_threads; t++) {
        for (uintE v = start[t]; v < end[t]; ++v) {
            if (in_degrees[v] > avgDegree) {
                local_degree_id_pairs[t].emplace_back(in_degrees[v], v);
            }
        }
    }
    for (int t = 0; t < num_threads; t++) {
        hub_count[t] = local_degree_id_pairs[t].size();
        hubCount += hub_count[t];
        non_hub_count[t] = end[t] - start[t] - hub_count[t];
    }
    new_index[0] = hubCount;
    for (int t = 1; t < num_threads; t++) {
        new_index[t] = new_index[t - 1] + non_hub_count[t - 1];
    }
    vector<degree_nodeid_t> degree_id_pairs(hubCount);
    
    long k = 0;
    for (int i = 0; i < num_threads; i++) {
        for (auto &j: local_degree_id_pairs[i]) {
            degree_id_pairs[k++] = j;
        }
        local_degree_id_pairs[i].clear();
    }
    assert(degree_id_pairs.size() == hubCount);
    assert(k == hubCount);
    
    __gnu_parallel::sort(degree_id_pairs.begin(), degree_id_pairs.end(),
                         std::greater<degree_nodeid_t>());

#pragma omp parallel for
    for (uintE n = 0; n < hubCount; ++n) {
        (*mapping_use)[degree_id_pairs[n].second] = n;
    }
    vector<degree_nodeid_t>().swap(degree_id_pairs);

#pragma omp parallel for schedule(static) num_threads(num_threads)
    for (uintE t = 0; t < num_threads; t++) {
        for (uintE v = start[t]; v < end[t]; ++v) {
            if ((*mapping_use)[v] == UINT_E_MAX) {
                (*mapping_use)[v] = new_index[t]++;
            }
        }
    }
    
    return mapping_use;
}

vector<vid_t> *generateRandomMapping(const vid_t nv, const eid_t ne,
                                     const vector<vid_t> &in_degrees,
                                     const vector<vid_t> &out_degrees) {
    auto *mapping_use = new vector<vid_t>(nv);
    
    uintE granularity = Config::rand_gran;
    uintE slice = (nv - granularity + 1) / granularity;
    uintE artificial_nv = slice * granularity;
    assert(artificial_nv <= nv);
    vector<vid_t> slice_index;
    slice_index.resize(slice);
    
    {
        parallel_for (long i = 0; i < slice; i++) {
            slice_index[i] = i;
        }
    }
    
    random_shuffle(slice_index.begin(), slice_index.end());
    
    {
        parallel_for (long i = 0; i < slice; i++) {
            long new_index = slice_index[i] * granularity;
            for (long j = 0; j < granularity; j++) {
                long v = (i * granularity) + j;
                if (v < artificial_nv) {
                    (*mapping_use)[v] = new_index + j;
                }
            }
        }
    }
    for (long i = artificial_nv; i < nv; i++) {
        (*mapping_use)[i] = i;
    }
    slice_index.clear();
    
    return mapping_use;
}

int verify_mapping(const vid_t nv, const eid_t ne,
                   const vector<vid_t> &in_degrees,
                   const vector<vid_t> &out_degrees,
                   const vector<vid_t> *new_ids) {
    auto *hist = new uintE[nv];
    {
        parallel_for (long i = 0; i < nv; i++) {
            hist[i] = (*new_ids)[i];
        }
    }
    
    __gnu_parallel::sort(&hist[0], &hist[nv]);
    
    uintE count = 0;
    {
        parallel_for (long i = 0; i < nv; i++) {
            if (hist[i] != i) {
                __sync_fetch_and_add(&count, 1);
            }
        }
    }
    if (count != 0) {
        cout << "Num of vertices did not match: " << count << std::endl;
        cout << "Mapping is invalid.!" << std::endl;
        abort();
    } else {
        cout << "Mapping is valid.!" << std::endl;
    }
    free(hist);
    return 0;
}

vector<vid_t> *generate_mapping(const vid_t nv, const eid_t ne,
                                const vector<vid_t> &in_degrees,
                                const vector<vid_t> &out_degrees) {
    Timer timer;
    timer.Start();
    vector<vid_t> *mapping_use;
    if (Config::name_reorder == EReorder::hubsort) {
        mapping_use = generateHubSortMapping(nv, ne, in_degrees, out_degrees);
    } else if (Config::name_reorder == EReorder::sort) {
        mapping_use = generateSortMapping(nv, ne, in_degrees, out_degrees);
    } else if (Config::name_reorder == EReorder::dbg) {
        mapping_use = generateDBGMapping(nv, ne, in_degrees, out_degrees);
    } else if (Config::name_reorder == EReorder::hubsortdbg) {
        mapping_use = generateHubSortDBGMapping(nv, ne, in_degrees, out_degrees);
    } else if (Config::name_reorder == EReorder::hubclusterdbg) {
        mapping_use = generateHubClusterDBGMapping(nv, ne, in_degrees, out_degrees);
    } else if (Config::name_reorder == EReorder::hubcluster) {
        mapping_use = generateHubClusterMapping(nv, ne, in_degrees, out_degrees);
    } else if (Config::name_reorder == EReorder::random) {
        mapping_use = generateRandomMapping(nv, ne, in_degrees, out_degrees);
    } else throw runtime_error("Unknown generate_mapping type");
    timer.Stop();
    timer.PrintSecond("generate_mapping");
    return mapping_use;
}

