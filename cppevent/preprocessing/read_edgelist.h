#ifndef preprocessing
#define preprocessing

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// #include <parallel/algorithm>
#include <omp.h>
#include <cassert>
#include <utility>

#include "../configs/timer.h"
#include "../configs/parallel.h"

#include "reordering/utils.h"
#include "reordering/reordering.h"
#include "reordering/sliding_queue.h"
#include "graph_loader.h"

using namespace std;

// A structure that keeps a sequence of strings all allocated from
// the same block of memory
struct words {
    long num_chars = 0; // total number of characters
    char *Chars{};  // array storing all strings
    long num_words{}; // number of substrings
    char **Strings{}; // pointers to strings (all should be nullptr terminated)
    words() = default;
    
    words(char *C, long nn, char **S, long mm)
            : Chars(C), num_chars(nn), Strings(S), num_words(mm) {}
    
    void del() const {
        free(Chars);
        free(Strings);
    }
};

inline bool isSpace(char c) {
    switch (c) {
        case '\r':
        case '\t':
        case '\n':
        case 0:
        case ' ' :
            return true;
        default :
            return false;
    }
}

struct COO {
public:
    vector<eid_t> src;
    vector<eid_t> dst;
    
    explicit COO(const eid_t ne) {
        src.resize(ne);
        dst.resize(ne);
    }
};

seqT<char> readStringFromFile(const string &fileName);
words stringToWords(char *Str, long n);
COO *load_coo(const string &, vid_t &, eid_t &,
              vector<vid_t> &, vector<vid_t> &);

#endif