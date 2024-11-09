#include <map>
#include <cstdio>

#include "hardware/helps.h"

using namespace std;

void start_run() {
    ArchTop *arch;
    arch = new ArchTop();
    
    printf("Simulation Starts.\n");
    while (!arch->check_flag_end()) {
        arch->tick();
    }
    printf("Simulation Ends.\n");
    helps::print_statistics(arch);
    cout << endl;
}

int main(int argc, char *const *argv) {
    Config(argc, argv);
    
    Timer timer;
    timer.Start();
    start_run();
    timer.Stop();
    timer.PrintSecond("Simulation Time (s): ");
    return 0;
}

