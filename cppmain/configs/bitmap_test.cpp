//
// Created by Flowerbeach on 2022/2/24.
//

#include "bitmap.h"
#include "../hardware/memory/database/csc_csr.h"

void test_bitmap_single_level() {
    auto b = BitmapSingle(8, 5, 0, 0);
    cout << "b.check_bit_1(0, 3): " << (unsigned int) b.array[0]
         << " " << b.check_bit_1(0, 3) << endl;
    // b.set_bit_1(0, 14);
    // b.set_bit_1(0, 15);
    b.set_bit_1(1, 3);
    b.print();
    cout << "b.check_bit_1(0, 3): " << (unsigned int) b.array[0]
         << " " << b.check_bit_1(0, 3) << endl;
    b.set_bit_1(6, 1);
    b.print();
    cout << "b.check_bit_1(0, 3): " << (unsigned int) b.array[0]
         << " " << b.check_bit_1(0, 3) << endl;
}

void test_bitmap_multiple_level() {
    BitmapMultiple::init_level(2, 16, 0);
    BitmapMultiple::init_level(8, 8, 1);
    auto mb = BitmapMultiple(256, 256);
    mb.print();
    mb.set_bit_1(16, 16);
    mb.print();
}

void test_bitmap_multiple_save_load() {
    BitmapMultiple::init_level(2, 16, 0);
    BitmapMultiple::init_level(8, 8, 1);
    auto mb = BitmapMultiple(256, 256);
    mb.print();
    mb.set_bit_1(16, 16);
    mb.print();
    mb.save();
    auto mb1 = BitmapMultiple();
    cout << endl;
    cout << endl;
    mb1.print();
}

void test_bitmap_single_save_load() {
    auto b = BitmapSingle(8, 8, 0, 0);
    experimental::filesystem::path target_path = fs::path("./test.txt");
    //         experimental::filesystem::path target_path = fs::path("../../profiling/bitmap");
    //         target_path /= dataset_to_string.at(Config::name_dataset) + string(".bitmap.txt");
    //         assert(fs::exists(target_path));
    ofstream outfile(target_path, ios::binary);
    
    b.set_bit_1(1, 3);
    b.print();
    b.save(outfile);
    outfile.close();
    
    cout << endl;
    cout << endl;
    
    auto a = BitmapSingle(8, 8, 0, 0);
    ifstream infile = ifstream(target_path, ios::binary);
    a.load(infile);
    a.print();
    infile.close();
}

int main() {
    test_bitmap_multiple_save_load();
}