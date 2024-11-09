//
// Created by Flowerbeach on 2021/7/24.
//

#ifndef CPPMAIN_CTRL_DECODE_H
#define CPPMAIN_CTRL_DECODE_H

#include <cassert>
#include <functional>
#include "stages.h"
#include "../../message/message.h"

class TileHub;
class ComputeUnit;

namespace control {
    class CtrlTop;
    
    class Decode : public PipeBase {
    protected:
        TileHub *th;
        Op *operation = nullptr;
        int parse_number(string str_number, tid_coalesced tids, vid_t pid);
        void parse_address(const string &str_address, int sid, buffer_addr_t &addr);
        void parse_instruction(stringstream &iss, int num_oper, int num_addr,
                               const tid_coalesced &tids, vid_t pid, int sid);
        void callback() override;
    public:
        explicit Decode(CtrlTop *sched, TileHub *th);
        bool decode(const string &instruction, const tid_coalesced &tids, vid_t pid, int sid);
        inline bool decode(const string &instruction, vid_t pid);
    };
}

inline bool control::Decode::decode(const string &instruction, vid_t pid) {
    return decode(instruction, {-1}, pid, Config::num_thread);
}

#endif //CPPMAIN_CTRL_DECODE_H
