#ifndef __REQUEST_H
#define __REQUEST_H

#include "../../../configs/shard.h"

#include <vector>
#include <functional>

// using namespace std;

typedef map<string, eid_t> tag_t;

class callback_args_t {
public:
    tag_t tag;
    Shard_T *shard = nullptr;
    callback_args_t() : tag({}) {}
    explicit callback_args_t(tag_t tag) : tag(std::move(tag)) {}
    callback_args_t(tag_t tag, Shard_T *shard) : tag(std::move(tag)), shard(shard) {}
};

typedef function<void(callback_args_t &)> callback_t;

namespace ramulator {
    class Request {
    public:
        bool is_first_command;
        long long addr = -1;
        // long addr_row;
        vector<int> addr_vec;
        
        enum class Type {
            READ, WRITE, REFRESH, POWERDOWN,
            SELFREFRESH, EXTENSION, MAX
        } type;
        
        long arrive = -1;
        long depart = -1;
        callback_args_t callback_args;
        callback_t callback = nullptr; // callback to buffer
        
        string name_buffer;
        int coreid = 0;
        
        void set_callback(callback_t cb, callback_args_t cba) {
            callback = std::move(cb);
            callback_args = std::move(cba);
        }
        
        Request(long long addr, Type type, string name_buffer)
                : is_first_command(true), addr(addr), type(type),
                  name_buffer(std::move(name_buffer)) {}
        Request(vector<int> &addr_vec, Type type, string name_buffer)
                : is_first_command(true), addr_vec(addr_vec), type(type),
                  name_buffer(std::move(name_buffer)) {}
        Request() : is_first_command(true) {}
    };
} /*namespace ramulator*/

#endif /*__REQUEST_H*/

