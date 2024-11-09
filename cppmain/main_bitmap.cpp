//
// Created by Flowerbeach on 2022/2/25.
//
#include "configs/bitmap.h"
#include "hardware/memory/database/csc_csr.h"

int main(int argc, char *argv[]) {
    Config(argc, argv);
    Timer timer;
    timer.Start();
    
    auto csc = CSC_CSR();
    BitmapMultiple::init_level(Config::bitmap_sizes[0].first,
                               Config::bitmap_sizes[0].second, 0);
    BitmapMultiple::init_level(Config::bitmap_sizes[1].first,
                               Config::bitmap_sizes[1].second, 1);
    auto bitmap = BitmapMultiple(csc.num_vertices, csc.num_edges, true);
    
    eid_t cnt = 0;
    for (int dst = 0; dst < csc.num_vertices; dst++) { ;
        for (auto offset = csc.csc_ptr_col[dst];
             offset < csc.csc_ptr_col[dst + 1]; offset++) {
            auto src = csc.csc_idx_row[offset];
            bitmap.set_bit_1(src, dst);
            cnt++;
        }
    }
    assert(cnt == csc.num_edges);
    bitmap.save();
    
    auto bitmap_load = BitmapMultiple();
    cout << "num_vertices loaded: " << bitmap_load.num_vertices << endl;
    cout << "num_edges loaded: " << bitmap_load.num_edges << endl;
    
    timer.Stop();
    timer.PrintSecond("Convertion Time (s): ");
    
}