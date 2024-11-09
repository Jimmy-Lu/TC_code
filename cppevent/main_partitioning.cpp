#include "main_partitioning.h"

using namespace std;

int main(int argc, char *argv[]) {
    Config(argc, argv);
    cout << "==================================" << endl;
    cout << "num_threads_partitioning: " << Config::num_workers << endl;
#ifdef OPENMP
    cout << "omp_get_max_threads: " << omp_get_max_threads() << endl;
#endif
    cout << "reordering_algo: " << reorder_to_string.at(Config::name_reorder) << endl;
    cout << "==================================" << endl;
    
    Timer timer;
    timer.Start();
    start_run();
    timer.Stop();
    timer.PrintSecond("Partition Time (s): ");
    return 0;
}

