//
// Created by SmartCabbage on 2021/2/2.
//
#include "config.h"
#include "argparse.h"
#include "bitmap.h"

using namespace std;

const int Config::FrequencyArch_MHz = 1024;
const int Config::BandwidthDRAM_GB_s = 256;  // GB/sec
const int Config::HBM_energy_pJ_bit = 7;    // pJ/bit
// const double Config::energy_op_normal_pJ = 0.991147114998765; // 16nm

// 32nm -> 16nm
// const double Config::power_leakage_mW = 11.0442 * 4 * 0.8 * 0.8; // 4 banks
// const double Config::energy_dynamic_nJ_feature = 0.322 * 0.569 * 0.569;

const int Config::SIZE_ELEMENT_BYTE = 4;
const int Config::SIZE_BANK_ROW_ELEMENT = 128;
int Config::SIZE_DST_MEMORY_MB = 8;
int Config::SIZE_SRC_MEMORY_MB = 1;

// const int Config::LATENCY_SRAM = 2.19096 * Config::FrequencyArch_MHz / 1024;
// const int Config::LATENCY_EB = 7.07888 * Config::FrequencyArch_MHz / 1024;
const int Config::LATENCY_SRAM = 1;
const int Config::LATENCY_EB = 1;

const double Config::POWER_MU_mW = double(1000 * 1.456) * double(Config::FrequencyArch_MHz) / 1024; // mW
const double Config::POWER_VU_mW = double(906.21312) * double(Config::FrequencyArch_MHz) / 1024; // mW
const double Config::POWER_CTRL_mW = double(161) * double(Config::FrequencyArch_MHz) / 1024; // mW
const double Config::POWER_TU_CSC_mW = double(72 + 4.435) * double(Config::FrequencyArch_MHz) / 1024; // mW
const double Config::POWER_TU_CSR_mW = double(4.435) * double(Config::FrequencyArch_MHz) / 1024; // mW
const double Config::POWER_TU_BITMAP_mW = double(-1) * double(Config::FrequencyArch_MHz) / 1024; // mW
const double Config::POWER_LEAKAGE_RAM_mW = double(3251.2); // mW
const double Config::ENERGY_DYNAMIC_EB_nJ = double(0); // nJ

const int Config::SIZE_FEATURE_ELEMENT = 128;
const int Config::SIZE_FEATURE_BYTE = Config::SIZE_FEATURE_ELEMENT
                                      * Config::SIZE_ELEMENT_BYTE;

// simulation parameters

EArch Config::name_arch = EArch::base;
EDataset Config::name_dataset = EDataset::cp;
ETiling Config::name_tiling = ETiling::sparse;
EReorder Config::name_reorder = EReorder::none;
EFormat Config::tile_format = EFormat::binary;
EModel Config::name_model = EModel::gat;
ETU Config::name_tu = ETU::none;
bool Config::vu_balance = false;
bool Config::coalesce = false;

long Config::num_element_feat_dst = -1;
long Config::num_element_feat_src = -1;
long Config::num_element_feat_edge = -1;

int Config::num_total_element_tile = -1;
int Config::tile_max_src_edge = -1;
int Config::tile_num_dst = -1;
int Config::num_vu = 1;
int Config::num_mu = 1;
int Config::num_tu = 8;
int Config::num_thread = 3;
int Config::num_edge_types = 3;

vector<pair<int, int>> Config::bitmap_sizes;

// filesystem path
fs::path Config::path_model;
fs::path Config::path_trash;

// graph reordering
int Config::rand_gran = 1;
int Config::num_workers = 4;

// DRAM ramulator
EHBMOrg Config::org = EHBMOrg::HBM_4Gb;
EHBMSpeed Config::speed = EHBMSpeed::HBM_1Gbps;
int Config::num_channels = -1;
int Config::num_ranks = -1;
int Config::bytes_dram_access_channel = 64; // byte
int Config::bytes_dram_access_dram = -1;
long long Config::bytes_dram_capacity = -1;

Config::Config(int argc, char *const *argv) {
    read_simulator_argument(argc, argv);
    update_simulator_config();
    read_ramulator_config();
    print_config();
}

void Config::update_simulator_config() {
    // feature size
    if (name_model == EModel::gcn) {
        num_element_feat_dst = Config::SIZE_FEATURE_ELEMENT;
        num_element_feat_src = Config::SIZE_FEATURE_ELEMENT;
        num_element_feat_edge = Config::SIZE_FEATURE_ELEMENT;
    } else if (name_model == EModel::gat) {
        num_element_feat_dst = (Config::SIZE_FEATURE_ELEMENT + 2);
        num_element_feat_src = (Config::SIZE_FEATURE_ELEMENT + 1);
        num_element_feat_edge = (2 + Config::SIZE_FEATURE_ELEMENT);
    } else if (name_model == EModel::sage) {
        num_element_feat_dst = (Config::SIZE_FEATURE_ELEMENT * 2);
        num_element_feat_src = Config::SIZE_FEATURE_ELEMENT;
        num_element_feat_edge = Config::SIZE_FEATURE_ELEMENT;
    } else if (name_model == EModel::ggnn) {
        num_element_feat_dst = Config::SIZE_FEATURE_ELEMENT * 4;
        num_element_feat_src = Config::SIZE_FEATURE_ELEMENT;
        num_element_feat_edge = Config::SIZE_FEATURE_ELEMENT;
    } else throw runtime_error("");
    
    tile_num_dst = int(double(SIZE_DST_MEMORY_MB) / SIZE_ELEMENT_BYTE * 1024 * 1024 / num_element_feat_dst);
    num_total_element_tile = int(double(SIZE_SRC_MEMORY_MB) / Config::SIZE_ELEMENT_BYTE * 1024 * 1024) / num_thread;
    tile_max_src_edge = ceil(double(num_total_element_tile) / Config::SIZE_FEATURE_ELEMENT);
    
    if (name_arch == EArch::tu) {
        name_tiling = ETiling::sparse;
        throw runtime_error("Is it necessary sparse?");
    } else if (name_arch == EArch::base) {
        num_tu = -1;
    } else throw runtime_error("name_arch\n");
    
    // filesystem path
    path_trash = "../../partition";
    path_model = "../algorithm/model/";
    path_model /= model_to_string.at(name_model) + ".model";
}

void Config::read_ramulator_config() {
    string filename = "../configs/ramulator-config/HBM-config.txt";
    ifstream file(filename);
    assert(file.good() && "Bad config file");
    string line;
    while (getline(file, line)) {
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
            line.erase(line.size() - 1);
        char delim[] = " = ";
        vector<string> tokens;
        
        while (true) {
            size_t start = line.find_first_not_of(delim);
            if (start == string::npos) break;
            
            size_t end = line.find_first_of(delim, start);
            if (end == string::npos) {
                tokens.push_back(line.substr(start));
                break;
            }
            tokens.push_back(line.substr(start, end - start));
            line = line.substr(end);
        }
        
        if (tokens.size() != 2) continue; // empty line
        if (tokens[0][0] == '#') continue; // comment line
        if (tokens[0] == "channels") num_channels = stoi(tokens[1]);
        else if (tokens[0] == "ranks") num_ranks = stoi(tokens[1]);
        
    }
    file.close();
    
    bytes_dram_capacity = (long long) (num_channels) * HBMOrg_to_MB.at(org) * 1024 * 1024;
    bytes_dram_access_dram = num_channels * bytes_dram_access_channel;
    
    bitmap_sizes.resize(BitmapMultiple::num_levels);
    bitmap_sizes[1].first = 1; // bit
    bitmap_sizes[0].second = 32; // bit
    // each bitmap has the size of channel transaction
    assert(bytes_dram_access_channel * 8 % bitmap_sizes[1].first == 0);
    assert(bytes_dram_access_channel * 8 % bitmap_sizes[0].second == 0);
    bitmap_sizes[0].first = bytes_dram_access_channel * 8 / bitmap_sizes[0].second;
    bitmap_sizes[1].second = bytes_dram_access_channel * 8 / bitmap_sizes[1].first;
}

void get(argparse::ArgumentParser &program, const string &arg, int &placeholder) {
    if (program.is_used(arg))
        placeholder = stoi(program.get<string>(arg));
}

void get(argparse::ArgumentParser &program, const string &arg, bool &placeholder) {
    if (program.is_used(arg)) {
        auto rst = stoi(program.get<string>(arg));
        if (rst == 1) placeholder = true;
        else if (rst == 0) placeholder = false;
        else throw runtime_error("invalid argument.");
    }
}

template<typename T>
void get(argparse::ArgumentParser &program, const string &arg, T &placeholder, const map<string, T> to_str) {
    if (program.is_used(arg))
        placeholder = to_str.at(program.get<string>(arg));
}

void Config::read_simulator_argument(int argc, char *const *argv) {
    argparse::ArgumentParser program("SGA");
    program.add_argument("-v", "--vu").help("number of vector units.");
    program.add_argument("-m", "--mu").help("number of matrix units.");
    program.add_argument("-p", "--tu").help("number of tiling units.");
    program.add_argument("-s", "--thread").help("number of edge/tile threads.");
    program.add_argument("-T", "--tu-type").help("dynamic tiling unit type.");
    
    program.add_argument("-S", "--size_seb").help("size of shard.");
    program.add_argument("-D", "--size_db").help("size of interval.");
    
    program.add_argument("-C", "--coalesce").help("enable row/tile coalesce.");
    program.add_argument("-V", "--vu-balance").help("enable vu intra-tile balance.");
    
    program.add_argument("-a", "--architecture").help("target accelerator version.");
    program.add_argument("-d", "--dataset").help("target graph dataset.");
    program.add_argument("-t", "--tiling").help("target graph tiling method.");
    program.add_argument("-r", "--reorder").help("target graph reorder method.");
    program.add_argument("-f", "--format").help("ascii(readable) or binary format.");
    program.add_argument("-k", "--model").help("target GNN model.");
    
    try { program.parse_args(argc, argv); }
    catch (const runtime_error &err) {
        cout << err.what() << endl << program;
        exit(0);
    }
    
    get(program, "-v", num_vu);
    get(program, "-m", num_mu);
    get(program, "-p", num_tu);
    get(program, "-s", num_thread);
    get(program, "-T", name_tu, string_to_tu);
    
    get(program, "-D", SIZE_DST_MEMORY_MB);
    get(program, "-S", SIZE_SRC_MEMORY_MB);
    
    get(program, "-C", coalesce);
    get(program, "-V", vu_balance);
    
    get(program, "-a", name_arch, string_to_arch);
    get(program, "-d", name_dataset, string_to_dataset);
    get(program, "-r", name_reorder, string_to_reorder);
    get(program, "-t", name_tiling, string_to_tiling);
    get(program, "-f", tile_format, string_to_format);
    get(program, "-k", name_model, string_to_model);
}

void Config::print_config() {
    cout << "==================================" << endl;
    printf("arch=%s, ", arch_to_string.at(Config::name_arch).c_str());
    printf("thread#=%s, ", to_string(num_thread).c_str());
    printf("vu#=%s, ", to_string(num_vu).c_str());
    printf("mu#=%s, ", to_string(num_mu).c_str());
    printf("tu#=%s.\n", to_string(num_tu).c_str());
    printf("coalesce=%d, ", int(coalesce));
    printf("vu-balance=%d\n", int(vu_balance));
    printf("------------------------\n");
    printf("dataset=%s, ", dataset_to_string.at(name_dataset).c_str());
    printf("tiling=%s, ", tiling_to_string.at(name_tiling).c_str());
    printf("format=%s.\n", format_to_string.at(tile_format).c_str());
    printf("t-element=%s, ", to_string(num_total_element_tile).c_str());
    printf("t-dst=%s.\n", to_string(tile_num_dst).c_str());
    printf("------------------------\n");
    printf("model=%s.\n", model_to_string.at(name_model).c_str());
    cout << "==================================" << endl;
}

