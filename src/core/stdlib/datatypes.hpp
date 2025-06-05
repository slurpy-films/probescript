#pragma once
#include "env.hpp"
#include "runtime/values.hpp"
#include "utils.hpp"
#include <string>

inline Val toNum(std::vector<Val> args, EnvPtr env) {
    if (!isNum(args[0]->toString())) {
        return env->throwErr("Cannot convert non number to number");
    }

    return std::make_shared<NumberVal>(std::to_string(args[0]->toNum()));
}

inline Val toStr(std::vector<Val> args, EnvPtr env) {
    if (args.empty()) return env->throwErr("");
    return std::make_shared<StringVal>(args[0]->toString());
}