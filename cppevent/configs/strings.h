//
// Created by SmartCabbage on 2021/1/27.
//

#ifndef CPPMAIN_STRINGS_H
#define CPPMAIN_STRINGS_H

#include <vector>
#include <string>
#include <map>

using namespace std;

///////////////////////////////
enum class EArch : int {
    base
};
const map<EArch, string> arch_to_string = {
        {EArch::base, "base"},
};
const map<string, EArch> string_to_arch = {
        {"base", EArch::base},
};

///////////////////////////////
enum class EPartitioning : int {
    baseline, hygcn, sparse,
};
const map<EPartitioning, string> partitioning_to_string = {
        {EPartitioning::sparse,   "sparse"},
        {EPartitioning::baseline, "baseline"},
        {EPartitioning::hygcn,    "hygcn"}};
const map<string, EPartitioning> string_to_partitioning = {
        {"sparse",   EPartitioning::sparse},
        {"baseline", EPartitioning::baseline},
        {"hygcn",    EPartitioning::hygcn}};

///////////////////////////////
enum class EDataset : int {
    cr, cs, pm,
    ax, yp, pd, fr, rd,
    ak, ad, cp, hw, sl,
};
const map<EDataset, string> dataset_to_string = {
        {EDataset::cr, "cr"},
        {EDataset::cs, "cs"},
        {EDataset::pm, "pm"},
        
        {EDataset::rd, "rd"},
        {EDataset::ax, "ax"},
        {EDataset::yp, "yp"},
        {EDataset::fr, "fr"},
        {EDataset::pd, "pd"},
        
        {EDataset::ak, "ak"},
        {EDataset::ad, "ad"},
        {EDataset::cp, "cp"},
        {EDataset::hw, "hw"},
        {EDataset::sl, "sl"},
};
const map<string, EDataset> string_to_dataset = {
        {"cr", EDataset::cr},
        {"cs", EDataset::cs},
        {"pm", EDataset::pm},
        
        {"rd", EDataset::rd},
        {"ax", EDataset::ax},
        {"yp", EDataset::yp},
        {"fr", EDataset::fr},
        {"pd", EDataset::pd},
        
        {"ak", EDataset::ak},
        {"ad", EDataset::ad},
        {"cp", EDataset::cp},
        {"hw", EDataset::hw},
        {"sl", EDataset::sl},
};
const map<EDataset, string> dict_source_dir = {
        {EDataset::ak, "../../profiling/datasets/ak2010.mtx"},
        {EDataset::ad, "../../profiling/datasets/coAuthorsDBLP.mtx"},
        {EDataset::cp, "../../profiling/datasets/directed/cit-Patents.csv"},
        {EDataset::hw, "../../profiling/datasets/undirected/hollywood.csv"},
        {EDataset::sl, "../../profiling/datasets/directed/soc-LiveJournal1.csv"},
        
        {EDataset::cr, "../../profiling/datasets/cora"},
        {EDataset::cs, "../../profiling/datasets/citeseer"},
        {EDataset::pm, "../../profiling/datasets/pubmed"},
        
        {EDataset::ax, "../../profiling/datasets/arxiv"},
        {EDataset::rd, "../../profiling/datasets/reddit"},
        {EDataset::yp, "../../profiling/datasets/yelp"},
        {EDataset::fr, "../../profiling/datasets/flickr"},
        {EDataset::pd, "../../profiling/datasets/products"},
};

enum class EReorder {
    none, // do not use reordering
    random, // validate shuffle
    sort, hubsort, hubcluster,
    dbg, hubsortdbg, hubclusterdbg, // degree based grouping
    metis,
};
const map<EReorder, string> reorder_to_string = {
        {EReorder::none,          "none"},
        {EReorder::sort,          "sort"},
        {EReorder::hubsort,       "hubsort"},
        {EReorder::hubcluster,    "hubcluster"},
        {EReorder::dbg,           "dbg"},
        {EReorder::hubsortdbg,    "hubsortdbg"},
        {EReorder::hubclusterdbg, "hubclusterdbg"},
        {EReorder::metis,         "metis"},
};
const map<string, EReorder> string_to_reorder = {
        {"none",          EReorder::none},
        {"sort",          EReorder::sort},
        {"hubsort",       EReorder::hubsort},
        {"hubcluster",    EReorder::hubcluster},
        {"dbg",           EReorder::dbg},
        {"hubsortdbg",    EReorder::hubsortdbg},
        {"hubclusterdbg", EReorder::hubclusterdbg},
        {"metis",         EReorder::metis},
};

///////////////////////////////
enum class EModel : int {
    gat, gcn, sage, ggnn, rgcn
};
const map<EModel, string> model_to_string = {
        {EModel::gat,  "gat"},
        {EModel::gcn,  "gcn"},
        {EModel::sage, "sage"},
        {EModel::ggnn, "ggnn"},
        {EModel::rgcn, "rgcn"}};
const map<string, EModel> string_to_model = {
        {"gat",  EModel::gat},
        {"gcn",  EModel::gcn},
        {"sage", EModel::sage},
        {"ggnn", EModel::ggnn},
        {"rgcn", EModel::rgcn}};

/////////////////////////////
enum class EFormat : int {
    binary, ascii
};
const map<EFormat, string> format_to_string = {
        {EFormat::binary, "binary"},
        {EFormat::ascii,  "ascii"}};
const map<string, EFormat> string_to_format = {
        {"binary", EFormat::binary},
        {"ascii",  EFormat::ascii}};

enum class EDevice : int {
    VU, MU, MEM, none
};
const map<EDevice, string> device_to_string = {
        {EDevice::VU,  "VU "},
        {EDevice::MU,  "MU "},
        {EDevice::MEM, "MEM"}};

enum class EHBMOrg : int {
    // per channel density here. Each stack comes with 8 channels
    HBM_1Gb, HBM_2Gb, HBM_4Gb, MAX
};

const map<EHBMOrg, int> HBMOrg_to_MB = {
        {EHBMOrg::HBM_1Gb, 128},
        {EHBMOrg::HBM_2Gb, 256},
        {EHBMOrg::HBM_4Gb, 512}};

enum class EHBMSpeed : int {
    HBM_1Gbps, MAX
};

#endif //CPPMAIN_STRINGS_H
