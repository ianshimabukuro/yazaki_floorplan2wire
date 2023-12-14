#pragma once

namespace ewd{
    enum Error{
        COMPUTE_NO_ERROR=0,
        INPUT_FORMAT_ERROR=1,       //数据格式错误
        INPUT_DATA_CONFLICT_ERROR=2,//数据内容错误，
        NO_SOLUTION_ERROR=3,        //无可行解错误
        OTHER_ERROR=100             //其它错误
    };
};

