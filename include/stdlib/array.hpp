#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include <string>

NativeFnValue* arraySize(string name) {
    return new NativeFnValue([name](vector<RuntimeVal*>, Env* env) -> RuntimeVal* {
        return new NumberVal(to_string(static_cast<ArrayVal*>(env->lookupVar(name))->items.size()));
    });
}

NativeFnValue* arrayPush(string name) {
    return new NativeFnValue([name](vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
        ArrayVal* array = static_cast<ArrayVal*>(env->lookupVar(name));
        for (RuntimeVal* arg : args) {
            array->items.push_back(arg);
        }
        return env->assignVar(name, array);
    });
}