#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include <string>

NativeFnValue* arraySize(std::string name) {
    return new NativeFnValue([name](std::vector<RuntimeVal*>, Env* env) -> RuntimeVal* {
        return new NumberVal(std::to_string(static_cast<ArrayVal*>(env->lookupVar(name))->items.size()));
    });
}

NativeFnValue* arrayPush(std::string name) {
    return new NativeFnValue([name](std::vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
        ArrayVal* array = static_cast<ArrayVal*>(env->lookupVar(name));
        for (RuntimeVal* arg : args) {
            array->items.push_back(arg);
        }
        return env->assignVar(name, array);
    });
}