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