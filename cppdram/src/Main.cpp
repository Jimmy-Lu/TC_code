#include "Config.h"
#include "Controller.h"
#include "Memory.h"
#include "DRAM.h"
#include "Statistics.h"
#include "Processor.h"
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>

/* Standards */
// #include "HBM.h"

using namespace std;
using namespace ramulator;

template<typename T>
void run_dramtrace(const Config &configs, Memory<T, Controller> &memory, const char *tracename) {

    /* initialize DRAM trace */
    Trace trace(tracename);

    /* run simulation */
    bool stall = false, end = false;
    int reads = 0, writes = 0, clks = 0;
    long addr = 0;
    Request::Type type = Request::Type::READ;
    map<int, int> latencies;
    auto read_complete = [&latencies](Request &r) { latencies[r.depart - r.arrive]++; };

    Request req(addr, type, read_complete);

    while (!end || memory.pending_requests()) {
        if (!end && !stall) {
            end = !trace.get_dramtrace_request(addr, type);
        }

        if (!end) {
            req.addr = addr;
            req.type = type;
            stall = !memory.send(req);
            if (!stall) {
                if (type == Request::Type::READ) reads++;
                else if (type == Request::Type::WRITE) writes++;
            }
        } else {
            memory.set_high_writeq_watermark(0.0f); // make sure that all write requests in the
            // write queue are drained
        }

        memory.tick();
        clks++;
        Stats::curTick++; // memory clock, global, for Statistics
    }
    // This a workaround for statistics set only initially lost in the end
    memory.finish();
    Stats::statlist.printall();

}

template<typename T>
void start_run(const Config &configs, T *spec, const vector<const char *> &files) {
    // initiate controller and memory
    int C = configs.get_channels(), R = configs.get_ranks();
    // Check and Set channel, rank number
    spec->set_channel_number(C);
    spec->set_rank_number(R);
    std::vector<Controller<T> *> ctrls;
    for (int c = 0; c < C; c++) {
        DRAM<T> *channel = new DRAM<T>(spec, configs.print, T::Level::Channel);
        channel->id = c;
        channel->regStats("");
        Controller<T> *ctrl = new Controller<T>(configs, channel);
        ctrls.push_back(ctrl);
    }
    Memory<T, Controller> memory(configs, ctrls);

    assert(files.size() != 0);
    run_dramtrace(configs, memory, files[0]);
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <configs-file> --mode=cpu,dram [--stats <filename>] <trace-filename1> <trace-filename2>\n"
               "Example: %s ramulator-configs.cfg --mode=cpu cpu.trace cpu.trace\n", argv[0], argv[0]);
        return 0;
    }

    Config configs(argv[1]);

    const std::string &standard = configs["standard"];
    assert(standard != "" || "DRAM standard should be specified.");

    const char *trace_type = strstr(argv[2], "=");
    trace_type++;
    configs.add("trace_type", "DRAM");

    int trace_start = 3;
    string stats_out;
    if (strcmp(argv[trace_start], "--stats") == 0) {
        Stats::statlist.output(argv[trace_start + 1]);
        stats_out = argv[trace_start + 1];
        trace_start += 2;
    } else {
        Stats::statlist.output(standard + ".stats");
        stats_out = standard + string(".stats");
    }

    // A separate file defines mapping for easy config.
    if (strcmp(argv[trace_start], "--mapping") == 0) {
        configs.add("mapping", argv[trace_start + 1]);
        trace_start += 2;
    } else {
        configs.add("mapping", "defaultmapping");
    }

    std::vector<const char *> files(&argv[trace_start], &argv[argc]);
    configs.set_core_num(argc - trace_start);

    HBM *hbm = new HBM(configs["org"], configs["speed"]);
    start_run(configs, hbm, files);

    printf("Simulation done. Statistics written to %s\n", stats_out.c_str());

    return 0;
}
