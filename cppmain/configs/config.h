//
// Created by SmartCabbage on 2021/1/27.
//

#ifndef CPPMAIN_CONFIG_H
#define CPPMAIN_CONFIG_H

// #define NDEBUG

#include <experimental/filesystem>

#include <map>
#include <deque>
#include <string>
#include <vector>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>

#include <cmath>
#include <utility>
#include <cassert>
#include <functional>

#include "strings.h"
#include "parallel.h"
#include "timer.h"

// #define _DEBUG_EXE

typedef int sid_t;
typedef int eid_t;
typedef int vid_t;
typedef eid_t tid_t;
typedef vid_t pid_t;

using namespace std;
namespace fs = std::experimental::filesystem;

class Config {
protected:
    static void read_ramulator_config();
    static void read_simulator_argument(int argc, char *const *argv);
    static void update_simulator_config();

public:
    Config(int argc, char *const *argv);
    static void print_config();
    
    // simulation parameters
    static EArch name_arch;
    static ETiling name_tiling;
    static EDataset name_dataset;
    static EModel name_model;
    static EFormat tile_format;
    static EReorder name_reorder;
    static ETU name_tu;
    static bool coalesce;
    static bool vu_balance;
    
    static long num_element_feat_dst;
    static long num_element_feat_src;
    static long num_element_feat_edge;
    
    static int num_total_element_tile;
    static int tile_max_src_edge;
    static int tile_num_dst;
    static int num_vu;
    static int num_mu;
    static int num_tu;
    static int num_thread;
    static int num_edge_types;
    
    static vector<pair<int, int>> bitmap_sizes;
    
    static fs::path path_model;
    static fs::path path_trash;
    
    // graph reordering
    static int rand_gran;
    static int num_workers;
    
    // DRAM ramulator
    static EHBMOrg org;
    static EHBMSpeed speed;
    static int num_ranks;
    static int num_channels;
    static int bytes_dram_access_channel;
    static int bytes_dram_access_dram;
    static long long bytes_dram_capacity;

public:
    // =========================
    // common configurations
    // =========================
    
    // accelerator
    static const int FrequencyArch_MHz;
    static const int BandwidthDRAM_GB_s;  // GB/sec
    static const int HBM_energy_pJ_bit;   // pJ/bit
    
    static const int SIZE_ELEMENT_BYTE;
    static const int SIZE_BANK_ROW_ELEMENT;
    static int SIZE_DST_MEMORY_MB;
    static int SIZE_SRC_MEMORY_MB;
    
    static const int SIZE_FEATURE_ELEMENT;
    static const int SIZE_FEATURE_BYTE;
    
    static const int LATENCY_SRAM;
    static const int LATENCY_EB;
    
    static const double POWER_MU_mW;
    static const double POWER_VU_mW;
    static const double POWER_CTRL_mW;
    static const double POWER_TU_CSC_mW;
    static const double POWER_TU_CSR_mW;
    static const double POWER_TU_BITMAP_mW;
    static const double POWER_LEAKAGE_RAM_mW;
    static const double ENERGY_DYNAMIC_EB_nJ;

public:
    template<class T>
    inline static bool exist(T val, vector<T> vec) {
        if (count(vec.begin(), vec.end(), val) > 0) return true;
        else return false;
    }
    
    template<typename T>
    inline static string vector_2_string(vector<T> vec, int size) {
        string ret;
        for (int i = 0; i < size; i++)
            ret.append(to_string(vec[i]) + " ");
        return ret;
    }
    
    template<typename T>
    inline static string vector_2_string(T *vec, int size) {
        string ret;
        for (int i = 0; i < size; i++)
            ret.append(to_string(vec[i]) + " ");
        return ret;
    }
    
    template<typename T>
    inline static void scalar_2_binary(const T &scalar, ofstream &outfile_b) {
        outfile_b.write((char *) &scalar, sizeof(T));
    }
    
    template<typename T>
    inline static void binary_2_scalar(T &scalar, ifstream &infile_b) {
        infile_b.read((char *) &scalar, sizeof(T));
    }
    
    inline static void string_2_binary(const string &str, ofstream &outfile_b) {
        outfile_b.write(str.c_str(), str.size() * sizeof(char));
    }
    
    inline static void binary_2_string(const string &str, ifstream &infile_b) {
        char str_read[100];
        infile_b.read(str_read, str.size() * sizeof(char));
        assert(str == string(str_read, str.size()));
    }
    
    inline static void binary_2_string(string &str, int size, ifstream &infile_b) {
        char str_read[100];
        infile_b.read(str_read, str.size() * sizeof(char));
        str = string(str_read, str.size());
    }
    
    template<typename T>
    inline static void vector_2_binary(T *vec, ofstream &outfile_b, int size_vec) {
        outfile_b.write((char *) &size_vec, sizeof(int));
        for (int i = 0; i < size_vec; i++)
            outfile_b.write((char *) &vec[i], sizeof(T));
    }
    
    template<typename T>
    inline static void vector_2_binary(vector<T> vec, ofstream &outfile_b, int size_vec) {
        outfile_b.write((char *) &size_vec, sizeof(int));
        for (int i = 0; i < size_vec; i++)
            outfile_b.write((char *) &vec[i], sizeof(T));
    }
    
    template<typename T>
    inline static void binary_2_vector(vector<T> &vec, ifstream &infile_b, int size_expected) {
        int size_vec;
        infile_b.read((char *) &size_vec, sizeof(int));
        assert(size_expected == size_vec);
        
        assert(vec.empty());
        vec.resize(size_vec);
        for (int i = 0; i < size_vec; i++)
            infile_b.read((char *) &vec[i], sizeof(T));
    }
    
    template<typename T>
    inline static void binary_2_vector(T *vec, ifstream &infile_b, int size_expected) {
        int size_file;
        infile_b.read((char *) &size_file, sizeof(int));
        assert(size_expected == size_file);
        
        assert(vec == nullptr);
        vec = new T[size_file];
        for (int i = 0; i < size_file; i++)
            infile_b.read((char *) &(vec[i]), sizeof(T));
    }
    
    template<typename T>
    inline static void ascii_2_vector(vector<T> vec, istringstream &iss_data, int size_expected) {
        int size_file;
        iss_data >> size_file;
        assert(size_file == size_expected);
        
        assert(vec.empty());
        vec.resize(size_file);
        for (int li = 0; li < size_file; li++)
            iss_data >> vec[li];
    }
    
    template<typename T>
    inline static void ascii_2_vector(T *vec, istringstream &iss_data, int size_expected) {
        int size_file;
        iss_data >> size_file;
        assert(size_file == size_expected);
        
        // assert(vec == nullptr);
        vec = new T[size_file];
        for (int li = 0; li < size_file; li++)
            iss_data >> vec[li];
    }
    
};

#endif //CPPMAIN_CONFIG_H

