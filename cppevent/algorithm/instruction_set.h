//
// Created by SmartCabbage on 2021/1/28.
//

#ifndef CPPMAIN_INSTRUCTION_SET_H
#define CPPMAIN_INSTRUCTION_SET_H

#include <map>
#include <vector>
#include <string>

using namespace std;

enum class ISA {
    ADD_SS, SUB_SS, MUL_SS, DIV_SS,
    ADD_VS, SUB_VS, MUL_VS, DIV_VS,
    ADD_VV_W, SUB_VV_W, MUL_VV_W, DIV_VV_W,
    ADD_VV_F, SUB_VV_F, MUL_VV_F, DIV_VV_F,
    EXP, TANH, RELU, SIGMOID,
    GEMV,
    
    GEMM, BMM,
    
    SCTR_F, SCTR_B,
    GTHR_F_SUM, GTHR_B_SUM,
    GTHR_F_MAX, GTHR_B_MAX,
    
    LD_SRC, LD_DST, ST_DST,
    
    SIGNAL_E, SIGNAL_V, WAIT,
    
    NONE,
};

const map<string, ISA> string_to_ISA = {
        {"ADD_SS",     ISA::ADD_SS},
        {"ADD_VS",     ISA::ADD_VS},
        {"ADD_VV_W",   ISA::ADD_VV_W},
        {"ADD_VV_F",   ISA::ADD_VV_F},
        {"SUB_SS",     ISA::SUB_SS},
        {"SUB_VS",     ISA::SUB_VS},
        {"SUB_VV_W",   ISA::SUB_VV_W},
        {"SUB_VV_F",   ISA::SUB_VV_F},
        {"MUL_SS",     ISA::MUL_SS},
        {"MUL_VS",     ISA::MUL_VS},
        {"MUL_VV_W",   ISA::MUL_VV_W},
        {"MUL_VV_F",   ISA::MUL_VV_F},
        {"DIV_SS",     ISA::DIV_SS},
        {"DIV_VS",     ISA::DIV_VS},
        {"DIV_VV_W",   ISA::DIV_VV_W},
        {"DIV_VV_F",   ISA::DIV_VV_F},
        
        {"EXP",        ISA::EXP},
        {"TANH",       ISA::TANH},
        {"RELU",       ISA::RELU},
        {"SIGMOID",    ISA::SIGMOID},
        {"GEMV",       ISA::GEMV},
        
        {"GEMM",       ISA::GEMM},
        {"BMM",        ISA::BMM},
        
        {"SCTR_F",     ISA::SCTR_F},
        {"SCTR_B",     ISA::SCTR_B},
        {"GTHR_F_SUM", ISA::GTHR_F_SUM},
        {"GTHR_B_SUM", ISA::GTHR_B_SUM},
        {"GTHR_F_MAX", ISA::GTHR_F_MAX},
        {"GTHR_B_MAX", ISA::GTHR_B_MAX},
        
        {"LD_SRC",     ISA::LD_SRC},
        {"LD_DST",     ISA::LD_DST},
        {"ST_DST",     ISA::ST_DST},
        
        {"SIGNAL_E",   ISA::SIGNAL_E},
        {"SIGNAL_V",   ISA::SIGNAL_V},
        {"WAIT",       ISA::WAIT},
        
        {"NONE",       ISA::NONE},
};

const map<ISA, string> ISA_to_string = {
        {ISA::ADD_SS,     "ADD_SS"},
        {ISA::ADD_VS,     "ADD_VS"},
        {ISA::ADD_VV_W,   "ADD_VV_W"},
        {ISA::ADD_VV_F,   "ADD_VV_F"},
        {ISA::SUB_SS,     "SUB_SS"},
        {ISA::SUB_VS,     "SUB_VS"},
        {ISA::SUB_VV_W,   "SUB_VV_W"},
        {ISA::SUB_VV_F,   "SUB_VV_F"},
        {ISA::MUL_SS,     "MUL_SS"},
        {ISA::MUL_VS,     "MUL_VS"},
        {ISA::MUL_VV_W,   "MUL_VV_W"},
        {ISA::MUL_VV_F,   "MUL_VV_F"},
        {ISA::DIV_SS,     "DIV_SS"},
        {ISA::DIV_VS,     "DIV_VS"},
        {ISA::DIV_VV_W,   "DIV_VV_W"},
        {ISA::DIV_VV_F,   "DIV_VV_F"},
        
        {ISA::EXP,        "EXP"},
        {ISA::TANH,       "TANH"},
        {ISA::RELU,       "RELU"},
        {ISA::SIGMOID,    "SIGMOID"},
        {ISA::GEMV,       "GEMV"},
        
        {ISA::GEMM,       "GEMM"},
        {ISA::BMM,        "BMM"},
        
        {ISA::SCTR_F,     "SCTR_F"},
        {ISA::SCTR_B,     "SCTR_B"},
        {ISA::GTHR_F_SUM, "GTHR_F_SUM"},
        {ISA::GTHR_B_SUM, "GTHR_B_SUM"},
        {ISA::GTHR_F_MAX, "GTHR_F_MAX"},
        {ISA::GTHR_B_MAX, "GTHR_B_MAX"},
        
        {ISA::LD_SRC,     "LD_SRC"},
        {ISA::LD_DST,     "LD_DST"},
        {ISA::ST_DST,     "ST_DST"},
        
        {ISA::SIGNAL_E,   "SIGNAL_E"},
        {ISA::SIGNAL_V,   "SIGNAL_V"},
        {ISA::WAIT,       "WAIT"},
        
        {ISA::NONE,       "NONE"},
};

const vector<ISA> LIST_OP_ASMD_S = {
        ISA::ADD_SS, ISA::ADD_VS,
        ISA::SUB_SS, ISA::SUB_VS,
        ISA::MUL_SS, ISA::MUL_VS,
        ISA::DIV_SS, ISA::DIV_VS};
const vector<ISA> LIST_OP_ASMD_V = {
        ISA::ADD_VV_W, ISA::ADD_VV_F,
        ISA::SUB_VV_W, ISA::SUB_VV_F,
        ISA::MUL_VV_W, ISA::MUL_VV_F,
        ISA::DIV_VV_W, ISA::DIV_VV_F};
const vector<ISA> LIST_OP_SPECIAL = {
        ISA::EXP, ISA::RELU, ISA::SIGMOID, ISA::TANH};
const vector<ISA> LIST_OP_SCATTER = {
        ISA::SCTR_F, ISA::SCTR_B};
const vector<ISA> LIST_OP_GATHER = {
        ISA::GTHR_F_SUM, ISA::GTHR_B_SUM,
        ISA::GTHR_F_MAX, ISA::GTHR_B_MAX};
const vector<ISA> LIST_OP_MEMORY = {
        ISA::LD_DST, ISA::ST_DST, ISA::LD_SRC};

// for decode
const vector<ISA> LIST_OP_ALU_1_3 = {
        ISA::ADD_SS, // LIST_OP_ASMD_S
        ISA::SUB_SS,
        ISA::MUL_SS,
        ISA::DIV_SS};
const vector<ISA> LIST_OP_ALU_2_3 = {
        ISA::ADD_VV_W, ISA::ADD_VV_F, ISA::ADD_VS, // LIST_OP_ASMD_V
        ISA::SUB_VV_W, ISA::SUB_VV_F, ISA::SUB_VS,
        ISA::MUL_VV_W, ISA::MUL_VV_F, ISA::MUL_VS,
        ISA::DIV_VV_W, ISA::DIV_VV_F, ISA::DIV_VS,
        ISA::GEMV}; // GEMV
const vector<ISA> LIST_OP_GOP_1_2 = {
        ISA::GTHR_F_SUM, ISA::GTHR_B_SUM, // LIST_OP_GATHER
        ISA::GTHR_F_MAX, ISA::GTHR_B_MAX,
        ISA::SCTR_F, ISA::SCTR_B,}; // LIST_OP_SCATTER
const vector<ISA> LIST_OP_MM_3_3 = {
        ISA::GEMM, ISA::BMM}; // MM
// const vector<ISA> LIST_OP_2_2 = {};

enum class ProgStage : int {
    scatter, gather, apply, none
};

const map<ProgStage, string> stage_to_string = {
        {ProgStage::scatter, "stage.scatter     "},
        {ProgStage::gather,  "stage.gather      "},
        {ProgStage::apply,   "stage.apply       "},
        {ProgStage::none,    "stage.none        "},
};

map<ProgStage, vector<string>> load_program();

#endif //CPPMAIN_INSTRUCTION_SET_H
