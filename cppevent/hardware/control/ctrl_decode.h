//
// Created by Flowerbeach on 2021/7/24.
//

#ifndef CPPMAIN_CTRL_DECODE_H
#define CPPMAIN_CTRL_DECODE_H

#include <cassert>
#include <functional>
#include <utility>
#include "../message/message.h"

class ShardHub;
class ComputeUnit;

namespace control {
    class CtrlTop;
    
    class Inst {
    public:
        string instruction;
        sid_coalesced shards_id;
        id_interval_t interval_id;
        id_thread_t thread_id;
        function<void(void)> callback;
        
        Inst(string instruction,
             sid_coalesced shards_id,
             id_interval_t interval_id,
             id_thread_t thread_id,
             function<void(void)> callback
        ) :
                interval_id(interval_id),
                thread_id(thread_id),
                shards_id(std::move(shards_id)),
                instruction(std::move(instruction)),
                callback(std::move(callback)) {}
    };
    
    class Decode {
    protected:
        ShardHub *sh = nullptr;
        CtrlTop *ctrl = nullptr;
        Op *operation = nullptr;
        int parse_number(string str_number, const sid_coalesced &sids, id_interval_t iid);
        void parse_address(const string &str_address, id_thread_t tid, buffer_addr_t &addr);
        void parse_instruction(stringstream &iss, int num_oper, int num_addr,
                               const sid_coalesced &sids, id_interval_t iid, id_thread_t tid);
        
        enum DecodeState {
            busy, ready
        } state = ready;
        int latency = 0;
    
    public:
        explicit Decode(CtrlTop *sched, ShardHub *th);
        bool decode(const Inst *inst);
        void tick();
    };
    
}

#endif //CPPMAIN_CTRL_DECODE_H
