#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "utils/isNum.hpp"
#include <string>

inline Val toNum(std::vector<Val> args, Env* env) {
    if (!isNum(args[0]->toString())) {
        return env->throwErr("Cannot convert non number to number");
    }

    return std::make_shared<NumberVal>(std::to_string(args[0]->toNum()));
}

inline Val toStr(std::vector<Val> args, Env* env) {
    if (args.empty()) return env->throwErr("");
    return std::make_shared<StringVal>(args[0]->toString());
}