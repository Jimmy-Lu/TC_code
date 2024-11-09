//
// Created by Flowerbeach on 2021/7/24.
//

#ifndef CPPMAIN_CTRL_FETCH_H
#define CPPMAIN_CTRL_FETCH_H

#include <cassert>
#include <functional>

#include "stages.h"
#include "../../message/message.h"

class ComputeUnit;

namespace control {
    class CtrlTop;
    
    class Fetch : public PipeBase {
    protected:
        string instruction;
        map<Stage, vector<string>> programs;
        void callback() override;
    public:
        explicit Fetch(CtrlTop *sched);
        bool fetch(Stage stage, int program_cnt, int sid);
    };
}
#endif //CPPMAIN_CTRL_FETCH_H
