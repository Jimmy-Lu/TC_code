//
// Created by flowe on 2021/7/5.
//
#include "read_edgelist.h"

COO *load_coo(const string &filename, vid_t &nv, eid_t &ne,
              vector<vid_t> &in_degrees,
              vector<vid_t> &out_degrees) {
    seqT<char> S = readStringFromFile(filename);
    words W = stringToWords(S.A, S.n);
    
    long len = W.num_words;
    nv = atoi(W.Strings[0]);
    ne = atoi(W.Strings[1]);
    if (len != ne * 2 + 2) {
        cout << "Bad input file" << endl;
        cout << "len: " << len << endl;
        cout << "num_v: " << nv << endl;
        cout << "num_e: " << ne << endl;
        abort();
    }
    
    auto *coo = new COO(ne);
    in_degrees.resize(nv);
    out_degrees.resize(nv);
    
    int num_threads = getWorkers();
    vector<vector<vid_t>> v_inter_out(num_threads, vector<vid_t>(nv));
    vector<vector<vid_t>> v_inter_in(num_threads, vector<vid_t>(nv));
#pragma omp parallel
    {
        int t = omp_get_thread_num();
        for (long e = t; e < ne; e += num_threads) {
            coo->src[e] = atol(W.Strings[2 * e + 2]);
            v_inter_out[t][coo->src[e]]++;
            coo->dst[e] = atol(W.Strings[2 * e + 3]);
            v_inter_in[t][coo->dst[e]]++;
        }
    }
    {
        parallel_for (long v = 0; v < nv; v++) {
            for (int t = 0; t < num_threads; t++) {
                out_degrees[v] += v_inter_out[t][v];
                in_degrees[v] += v_inter_in[t][v];
            }
        }
    }
    
    for (int t = 0; t < num_threads; t++) {
        v_inter_in[t].clear();
        v_inter_out[t].clear();
    }
    v_inter_out.clear();
    v_inter_in.clear();
    W.del(); // to deal with performance bug in malloc
    
    cout << "Read directed graph." << endl;
    cout << "Node#=" << nv << ",   Edge#=" << ne << "." << endl;
    cout << "==================================" << endl;
    
    return coo;
}

// parallel code for converting a string to words
words stringToWords(char *Str, long n) {
    {
        parallel_for (long i = 0; i < n; i++)if (isSpace(Str[i])) Str[i] = 0;
    }
    
    // mark start of words
    bool *FL = newA(bool, n);
    assert(FL != nullptr && "Malloc failure\n");
    FL[0] = Str[0];
    { parallel_for (long i = 1; i < n; i++) FL[i] = Str[i] && !Str[i - 1]; }
    
    // offset for each start of word
    seqT<long> Off = sequence::packIndex<long>(FL, n);
    free(FL);
    long m = Off.n;
    long *offsets = Off.A;
    
    // pointer to each start of word
    char **SA = newA(char*, m);
    assert(SA != nullptr && "Malloc failure\n");
    { parallel_for (long j = 0; j < m; j++) SA[j] = Str + offsets[j]; }
    
    free(offsets);
    return words(Str, n, SA, m);
}

seqT<char> readStringFromFile(const string &fileName) {
    ifstream file(fileName, ios::in | ios::binary | ios::ate);
    if (!file.is_open()) {
        std::cout << "Unable to open file: " << fileName << std::endl;
        abort();
    }
    long end = file.tellg();
    file.seekg(0, ios::beg);
    long n = end - file.tellg();
    char *bytes = newA(char, n + 1);
    assert(bytes != nullptr && "Malloc failure\n");
    file.read(bytes, n);
    file.close();
    return seqT<char>(bytes, n);
}
