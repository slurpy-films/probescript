#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include <string>
#include <unordered_map>
#include <memory>

std::unordered_map<std::string, Val> getProcessModule() {
    std::unordered_map<std::string, Val> promod = {
        {
            "kill",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
                exit(std::static_pointer_cast<NumberVal>(args.size() >= 1 ? args[0] : 0)->toNum());
                return std::make_shared<UndefinedVal>();
            })
        }
    };

    return promod;
};