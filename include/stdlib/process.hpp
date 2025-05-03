#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include <string>
#include <unordered_map>


std::unordered_map<std::string, RuntimeVal*> processModule = {
    { 
        "kill",
        new NativeFnValue([](std::vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
            exit(static_cast<NumberVal*>(args.size() >= 1 ? args[0] : 0)->toNum());
            return new UndefinedVal();
        })
    }
};