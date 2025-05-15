#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include <string>

inline std::shared_ptr<NativeFnValue> arraySize(std::string name) {
    return std::make_shared<NativeFnValue>([name](std::vector<Val>, Env* env) -> Val {
        return std::make_shared<NumberVal>(std::to_string(std::static_pointer_cast<ArrayVal>(env->lookupVar(name))->items.size()));
    });
}

inline std::shared_ptr<NativeFnValue> arrayPush(std::string name) {
    return std::make_shared<NativeFnValue>([name](std::vector<Val> args, Env* env) -> Val {
        std::shared_ptr<ArrayVal> array = std::static_pointer_cast<ArrayVal>(env->lookupVar(name));
        for (Val arg : args) {
            array->items.push_back(arg);
        }
        return env->assignVar(name, array);
    });
}