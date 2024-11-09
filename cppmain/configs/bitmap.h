//
// Created by Flowerbeach on 2022/2/24.
//

#include <cassert>
#include <cstring>
#include <utility>

#include "tile.h"

#define BITMAP_BINARY 1

#ifndef CPPMAIN_CONFIG_BITMAP_H
#define CPPMAIN_CONFIG_BITMAP_H

class BitmapSingle {
public:
    typedef unsigned char element_type;
    int element_bytes = sizeof(element_type);
    int element_bits = element_bytes * 8;
    
    eid_t id_linear;
    std::pair<int, int> size;
    std::pair<int, int> id;
    element_type *array;
    int size_e;
    
    BitmapSingle(int size_x, int size_y, id_t id_x, vid_t id_y, eid_t i = -1) {
        assert(size_x * size_y % element_bits == 0);
        size_e = size_x * size_y / element_bits;
        size.first = size_x;
        size.second = size_y;
        
        id.first = id_x;
        id.second = id_y;
        id_linear = i;
        
        array = new element_type[size_e];
        memset(array, 0, size_e * element_bytes);
    }
    
    ~BitmapSingle() { delete[] array; }
    
    inline bool check_argument(int x, int y) const {
        if (x < size.first && y < size.second) return true;
        else return false;
    }
    inline bool check_bit_1(int x, int y) const {
        assert(check_argument(x, y));
        int bits = (x * size.second + y);
        int i = bits / element_bits;
        int offset = element_bits - bits % element_bits - 1;
        return array[i] >> offset & 1U;
    }
    inline void set_bit_1(int x, int y) const {
        assert(check_argument(x, y));
        int bits = (x * size.second + y);
        int i = bits / element_bits;
        int offset = element_bits - bits % element_bits - 1;
        array[i] = array[i] | 1U << offset;
    }
    inline void set_bit_0(int x, int y) const {
        assert(check_argument(x, y));
        int bits = (x * size.second + y);
        int i = bits / element_bits;
        int offset = element_bits - bits % element_bits - 1;
        array[i] = array[i] & ~(1U << offset);
    }
    inline void print() const {
        int cnt = 0;
        for (int i = 0; i < size_e; i++)
            for (int b = element_bits - 1; 0 <= b; b--) {
                printf("%c", (array[i] & (1 << b)) ? '1' : '0');
                if (++cnt % size.second == 0) {
                    cout << endl;
                    cnt = 0;
                }
            }
    }
    inline void save(ofstream &outfile) const {
#ifdef BITMAP_BINARY
        Config::scalar_2_binary(id_linear, outfile);
        outfile.write((char *) array, size_e * sizeof(element_type));
#else
        outfile << id_linear << " " << size_e << " "
                << Config::vector_2_string(array, size_e).c_str() << "\n";
#endif
    }
    inline void load(ifstream &infile) {
#ifdef BITMAP_BINARY
        Config::binary_2_scalar(id_linear, infile);
        infile.read((char *) array, size_e * sizeof(element_type));
#else
        string line;
        getline(infile, line);
        istringstream iss_data(line);
        iss_data >> id_linear;
        Config::ascii_2_vector(array, iss_data, size_e);
#endif
    }
};

class BitmapMultiple {
public:
    const static int num_levels = 2;
    static pair<int, int> sizes[num_levels];
    typedef vector<vector<BitmapSingle *>> level_t;
    vector<level_t> levels;
    pair<vid_t, vid_t> size_target;
    vector<eid_t> num_allocated;
    
    vid_t num_vertices = -1;
    eid_t num_edges = -1;
    
    BitmapMultiple() { load(); };
    BitmapMultiple(vid_t num_v, eid_t num_e, bool graph) {
        init_bitmap(num_v, num_v);
        num_vertices = num_v;
        num_edges = num_e;
    }
    BitmapMultiple(vid_t total_x, vid_t total_y) {
        init_bitmap(total_x, total_y);
    }
    void init_bitmap(vid_t total_x, vid_t total_y) {
        size_target.first = total_x;
        size_target.second = total_y;
        
        levels.resize(num_levels);
        num_allocated.assign(num_levels, 0);
        for (int i = num_levels - 1; i >= 0; i--) {
            vid_t num_rows = ceil(double(total_x) / sizes[i].first);
            vid_t num_cols = ceil(double(total_y) / sizes[i].second);
            levels[i].resize(num_rows);
            for (int r = 0; r < num_rows; r++) {
                levels[i][r].resize(num_cols);
            }
            total_x = num_rows;
            total_y = num_cols;
        }
    }
    inline static void init_level(int size_x, int size_y, int l) {
        sizes[l].first = size_x;
        sizes[l].second = size_y;
    }
    ~BitmapMultiple() {
        for (int l = 0; l < num_levels; l++)
            for (int r = 0; r < sizes[l].first; r++)
                for (int c = 0; c < sizes[l].second; c++)
                    delete levels[l][r][c];
    }
    
    inline void set_bit_1(int x, int y) {
        for (int i = num_levels - 1; i >= 0; i--) {
            vid_t r = x / sizes[i].first;
            vid_t c = y / sizes[i].second;
            if (levels[i][r][c] == nullptr) {
                levels[i][r][c] = new BitmapSingle(sizes[i].first, sizes[i].second, r, c,
                                                   num_allocated[0] + num_allocated[1]);
                num_allocated[i]++;
            }
            levels[i][r][c]->set_bit_1(x % sizes[i].first, y % sizes[i].second);
            x = r;
            y = c;
        }
    }
    inline void print() const {
        auto total_x = size_target.first;
        auto total_y = size_target.second;
        
        for (int i = num_levels - 1; i >= 0; i--) {
            vid_t num_rows = ceil(double(total_x) / sizes[i].first);
            vid_t num_cols = ceil(double(total_y) / sizes[i].second);
            for (int r = 0; r < num_rows; r++) {
                for (int c = 0; c < num_cols; c++) {
                    if (levels[i][r][c] == nullptr) continue;
                    printf("L%d, %d-%d\n", i, r, c);
                    levels[i][r][c]->print();
                }
            }
            total_x = num_rows;
            total_y = num_cols;
        }
    }
    inline void save() const {
        // experimental::filesystem::path target_path = fs::path("./test.txt");
        experimental::filesystem::path target_path = fs::path("../../profiling/bitmap");
        if (!fs::exists(target_path)) fs::create_directory(target_path);
        target_path /= dataset_to_string.at(Config::name_dataset) + string(".bitmap.txt");
#ifdef BITMAP_BINARY
        ofstream outfile(target_path, ios::binary);
#else
        ofstream outfile(target_path);
#endif

#ifdef BITMAP_BINARY
        Config::scalar_2_binary(num_vertices, outfile);
        Config::scalar_2_binary(num_edges, outfile);
        Config::string_2_binary("total_x_y", outfile);
        Config::scalar_2_binary(size_target.first, outfile);
        Config::scalar_2_binary(size_target.second, outfile);
#else
        outfile << num_vertices << " "
                << num_edges << " ";
        outfile << "total_x_y "
                << size_target.first << " "
                << size_target.second << "\n";
#endif
        auto total_x = size_target.first;
        auto total_y = size_target.second;
        
        for (int i = num_levels - 1; i >= 0; i--) {
#ifdef BITMAP_BINARY
            Config::string_2_binary("size_x_y", outfile);
            Config::scalar_2_binary(sizes[i].first, outfile);
            Config::scalar_2_binary(sizes[i].second, outfile);
#else
            outfile << "size_x_y "
                    << sizes[i].first << " "
                    << sizes[i].second << "\n";
#endif
            vid_t num_rows = ceil(double(total_x) / sizes[i].first);
            vid_t num_cols = ceil(double(total_y) / sizes[i].second);
            
            for (int r = 0; r < num_rows; r++) {
                for (int c = 0; c < num_cols; c++) {
                    if (levels[i][r][c] == nullptr) continue;
#ifdef BITMAP_BINARY
                    Config::string_2_binary("y", outfile);
                    Config::string_2_binary("id_x_y", outfile);
                    Config::scalar_2_binary(r, outfile);
                    Config::scalar_2_binary(c, outfile);
#else
                    outfile << "y\nid_x_y " << r << " " << c << "\n";
#endif
                    levels[i][r][c]->save(outfile);
                }
            }

#ifdef BITMAP_BINARY
            Config::string_2_binary("n", outfile);
#else
            outfile << "n\n";
#endif
            total_x = num_rows;
            total_y = num_cols;
        }
        
        outfile.close();
    }
    inline void load() {
        // experimental::filesystem::path target_path = fs::path("./test.txt");
        experimental::filesystem::path target_path = fs::path("../../profiling/bitmap");
        target_path /= dataset_to_string.at(Config::name_dataset) + string(".bitmap.txt");
        assert(fs::exists(target_path));
#ifdef BITMAP_BINARY
        ifstream infile = ifstream(target_path, ios::binary);
#else
        string line, name;
        ifstream infile = ifstream(target_path);
#endif

#ifdef BITMAP_BINARY
        Config::binary_2_scalar(num_vertices, infile);
        Config::binary_2_scalar(num_edges, infile);
        Config::binary_2_string("total_x_y", infile);
        Config::binary_2_scalar(size_target.first, infile);
        Config::binary_2_scalar(size_target.second, infile);
#else
        getline(infile, line);
        istringstream iss(line);
        iss >> num_vertices >> num_edges >> name >> size_target.first >> size_target.second;
        assert(name == "total_x_y");
#endif
        auto total_x = size_target.first;
        auto total_y = size_target.second;
        
        levels.resize(num_levels);
        num_allocated.assign(num_levels, 0);
        for (int i = num_levels - 1; i >= 0; i--) {
#ifdef BITMAP_BINARY
            Config::binary_2_string("size_x_y", infile);
            Config::binary_2_scalar(sizes[i].first, infile);
            Config::binary_2_scalar(sizes[i].second, infile);
#else
            getline(infile, line);
            istringstream iss_size(line);
            iss_size >> name >> sizes[i].first >> sizes[i].second;
            assert(name == "size_x_y");
#endif
            vid_t num_rows = ceil(double(total_x) / sizes[i].first);
            vid_t num_cols = ceil(double(total_y) / sizes[i].second);
            levels[i].resize(num_rows);
            for (int r = 0; r < num_rows; r++)
                levels[i][r].resize(num_cols);
            
            while (true) {
                int c = -1, r = -1;
                string identifier = "x";
#ifdef BITMAP_BINARY
                Config::binary_2_string(identifier, 0, infile);
#else
                getline(infile, line);
                istringstream iss_y(line);
                iss_y >> identifier;
#endif
                if (identifier[0] == 'n') break;
                assert(identifier[0] == 'y');
                num_allocated[i]++;

#ifdef BITMAP_BINARY
                Config::binary_2_string("id_x_y", infile);
                Config::binary_2_scalar(r, infile);
                Config::binary_2_scalar(c, infile);
#else
                getline(infile, line);
                istringstream iss_id(line);
                iss_id >> name >> r >> c;
                assert(name == "id_x_y");
#endif
                levels[i][r][c] = new BitmapSingle(
                        sizes[i].first, sizes[i].second, r, c);
                levels[i][r][c]->load(infile);
            }
            
            total_x = num_rows;
            total_y = num_cols;
        }
        
        infile.close();
    }
};

#endif //CPPMAIN_CONFIG_BITMAP_H
