#ifndef __PROCESSOR_H
#define __PROCESSOR_H

// #include "Cache.h"
#include "Config.h"
#include "Memory.h"
#include "Request.h"
#include "Statistics.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <ctype.h>
#include <functional>

namespace ramulator {

    class Trace {
    public:
        Trace(const char *trace_fname);
        // [address(hex)] [R/W]
        bool get_dramtrace_request(long &req_addr, Request::Type &req_type);

    private:
        std::ifstream file;
    };

    Trace::Trace(const char *trace_fname) : file(trace_fname) {
        if (!file.good()) {
            std::cerr << "Bad trace file: " << trace_fname << std::endl;
            exit(1);
        }
    }

    bool Trace::get_dramtrace_request(long &req_addr, Request::Type &req_type) {
        string line;
        getline(file, line);
        if (file.eof()) {
            return false;
        }
        size_t pos;
        req_addr = std::stoul(line, &pos, 16);

        pos = line.find_first_not_of(' ', pos + 1);

        if (pos == string::npos || line.substr(pos)[0] == 'R')
            req_type = Request::Type::READ;
        else if (line.substr(pos)[0] == 'W')
            req_type = Request::Type::WRITE;
        else
            assert(false);
        return true;
    }

}

#endif /* __PROCESSOR_H */
