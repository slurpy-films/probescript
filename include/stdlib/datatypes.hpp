#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include <string>

RuntimeVal* toNum(vector<RuntimeVal*> args, Env* env) {
    return new NumberVal(to_string(args[0]->toNum()));
}

RuntimeVal* toStr(vector<RuntimeVal*> args, Env* env) {
    return new StringVal(args[0]->toString());
}