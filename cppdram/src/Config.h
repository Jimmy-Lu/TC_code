#ifndef __CONFIG_H
#define __CONFIG_H

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>
#include <cassert>

namespace ramulator {
    // using namespace std;

    class Config {

    private:
        std::map<std::string, std::string> options;
        int channels;
        int ranks;
        int subarrays;
        int cpu_tick;
        int mem_tick;
        int core_num = 0;
        long expected_limit_insts = 0;
        long warmup_insts = 0;

    public:
        bool print = false;

        Config() {}
        Config(const std::string &fname);
        void parse(const std::string &fname);
        std::string operator[](const std::string &name) const {
            if (options.find(name) != options.end()) {
                return (options.find(name))->second;
            } else {
                return "";
            }
        }

        bool contains(const std::string &name) const {
            if (options.find(name) != options.end()) {
                return true;
            } else {
                return false;
            }
        }

        void add(const std::string &name, const std::string &value) {
            if (!contains(name)) {
                options.insert(make_pair(name, value));
            } else {
                printf("ramulator::Config::add options[%s] already set.\n", name.c_str());
            }
        }

        void set_core_num(int _core_num) { core_num = _core_num; }

        int get_channels() const { return channels; }
        int get_subarrays() const { return subarrays; }
        int get_ranks() const { return ranks; }
        int get_cpu_tick() const { return cpu_tick; }
        int get_mem_tick() const { return mem_tick; }
        int get_core_num() const { return core_num; }
        long get_expected_limit_insts() const { return expected_limit_insts; }
        long get_warmup_insts() const { return warmup_insts; }

        bool is_early_exit() const {
            // the default value is true
            if (options.find("early_exit") != options.end()) {
                if ((options.find("early_exit"))->second == "off") {
                    return false;
                }
                return true;
            }
            return true;
        }
        bool calc_weighted_speedup() const {
            return (expected_limit_insts != 0);
        }
        bool record_cmd_trace() const {
            // the default value is false
            if (options.find("record_cmd_trace") != options.end()) {
                if ((options.find("record_cmd_trace"))->second == "on") {
                    return true;
                }
                return false;
            }
            return false;
        }
        bool print_cmd_trace() const {
            // the default value is false
            if (options.find("print_cmd_trace") != options.end()) {
                if ((options.find("print_cmd_trace"))->second == "on") {
                    return true;
                }
                return false;
            }
            return false;
        }
    };

    Config::Config(const std::string &fname) {
        parse(fname);
    }

    void Config::parse(const std::string &fname) {
        std::ifstream file(fname);
        assert(file.good() && "Bad config file");
        std::string line;
        while (getline(file, line)) {
            char delim[] = " \t=";
            std::vector<std::string> tokens;

            while (true) {
                size_t start = line.find_first_not_of(delim);
                if (start == std::string::npos)
                    break;

                size_t end = line.find_first_of(delim, start);
                if (end == std::string::npos) {
                    tokens.push_back(line.substr(start));
                    break;
                }

                tokens.push_back(line.substr(start, end - start));
                line = line.substr(end);
            }

            // empty line
            if (!tokens.size())
                continue;

            // comment line
            if (tokens[0][0] == '#')
                continue;

            // parameter line
            assert(tokens.size() == 2 && "Only allow two tokens in one line");

            options[tokens[0]] = tokens[1];

            if (tokens[0] == "channels") {
                channels = atoi(tokens[1].c_str());
            } else if (tokens[0] == "ranks") {
                ranks = atoi(tokens[1].c_str());
            } else if (tokens[0] == "subarrays") {
                subarrays = atoi(tokens[1].c_str());
            } else if (tokens[0] == "cpu_tick") {
                cpu_tick = atoi(tokens[1].c_str());
            } else if (tokens[0] == "mem_tick") {
                mem_tick = atoi(tokens[1].c_str());
            } else if (tokens[0] == "expected_limit_insts") {
                expected_limit_insts = atoi(tokens[1].c_str());
            } else if (tokens[0] == "warmup_insts") {
                warmup_insts = atoi(tokens[1].c_str());
            }
        }
        file.close();
    }

} /* namespace ramulator */

#endif /* _CONFIG_H */

