//
// Created by Flowerbeach on 2021/2/12.
//

#include "../configs/config.h"
#include "instruction_set.h"

map<Stage, vector<string>> load_program() {
    map<Stage, vector<string>> program_insts;
    
    ifstream in(Config::path_model);
    if (!in.is_open()) throw runtime_error("bad model program.\n");
    
    Stage stage_ptr = Stage::None;
    bool found_scatter = false;
    bool found_gather = false;
    bool found_simple = false;
    bool found_apply = false;
    
    string line;
    while (getline(in, line)) {
        if (line[0] == '#') continue;
        while (line.front() == ' ') line.erase(0, 1);
        while ((line.back() == '\n'
                || line.back() == '\r'
                || line.back() == ' ')
               && !line.empty())
            line.erase(line.size() - 1);
        if (line.empty()) continue;
        
        // decode
        if (line == "Scatter") {
            assert(!found_scatter);
            found_scatter = true;
            stage_ptr = Stage::Scatter;
        } else if (line == "Gather") {
            assert(!found_gather);
            found_gather = true;
            stage_ptr = Stage::Gather;
        } else if (line == "GatherSimple") {
            assert(!found_simple);
            found_simple = true;
            stage_ptr = Stage::GatherSimple;
        } else if (line == "Apply") {
            assert(!found_apply);
            found_apply = true;
            stage_ptr = Stage::Apply;
        } else if (stage_ptr != Stage::None) {
            program_insts[stage_ptr].push_back(line);
        } else throw runtime_error(line + ": loading program.");
    }
    
    return program_insts;
}

