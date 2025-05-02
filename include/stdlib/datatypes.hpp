#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include <string>

RuntimeVal* toNum(std::vector<RuntimeVal*> args, Env* env) {
    return new NumberVal(std::to_string(args[0]->toNum()));
}

RuntimeVal* toStr(std::vector<RuntimeVal*> args, Env* env) {
    return new StringVal(args[0]->toString());
}