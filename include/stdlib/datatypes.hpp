#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "utils/isNum.hpp"
#include <string>

Val toNum(std::vector<Val> args, Env* env) {
    if (!isNum(args[0]->toString())) {
        std::cerr << "Cannot convert non number to number";
        exit(1);
    }

    return std::make_shared<NumberVal>(std::to_string(args[0]->toNum()));
}

Val toStr(std::vector<Val> args, Env* env) {
    return std::make_shared<StringVal>(args[0]->toString());
}