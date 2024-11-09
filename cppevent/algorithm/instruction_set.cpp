//
// Created by Flowerbeach on 2021/2/12.
//

#include "../configs/config.h"
#include "instruction_set.h"

map<ProgStage, vector<string>> load_program() {
    map<ProgStage, vector<string>> program;
    
    ifstream in(Config::path_model);
    if (!in.is_open()) throw runtime_error("bad model program.\n");
    
    ProgStage stage_ptr = ProgStage::none;
    bool found_scatter = false;
    bool found_gather = false;
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
            stage_ptr = ProgStage::scatter;
        } else if (line == "Gather") {
            assert(!found_gather);
            found_gather = true;
            stage_ptr = ProgStage::gather;
        } else if (line == "Apply") {
            assert(!found_apply);
            found_apply = true;
            stage_ptr = ProgStage::apply;
        } else if (stage_ptr != ProgStage::none) {
            program[stage_ptr].push_back(line);
        } else throw runtime_error(line + ": loading program.");
    }
    
    return program;
}

