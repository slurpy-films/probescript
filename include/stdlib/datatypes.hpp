#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "utils/isNum.hpp"
#include <string>

RuntimeVal* toNum(std::vector<RuntimeVal*> args, Env* env) {
    if (!isNum(args[0]->toString())) {
        std::cerr << "Cannot convert non number to number";
        exit(1);
    }

    return new NumberVal(std::to_string(args[0]->toNum()));
}

RuntimeVal* toStr(std::vector<RuntimeVal*> args, Env* env) {
    return new StringVal(args[0]->toString());
}