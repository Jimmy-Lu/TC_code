#include "Controller.h"
#include "SALP.h"
#include "ALDRAM.h"
#include "TLDRAM.h"

using namespace ramulator;

namespace ramulator {
    
    static vector<int> get_offending_subarray(DRAM<SALP> *channel, vector<int> &addr_vec) {
        int sa_id = 0;
        auto rank = channel->children[addr_vec[int(SALP::Level::Rank)]];
        auto bank = rank->children[addr_vec[int(SALP::Level::Bank)]];
        auto sa = bank->children[addr_vec[int(SALP::Level::SubArray)]];
        for (auto sa_other: bank->children)
            if (sa != sa_other && sa_other->state == SALP::State::Opened) {
                sa_id = sa_other->id;
                break;
            }
        vector<int> offending = addr_vec;
        offending[int(SALP::Level::SubArray)] = sa_id;
        offending[int(SALP::Level::Row)] = -1;
        return offending;
    }
    
} /* namespace ramulator */
