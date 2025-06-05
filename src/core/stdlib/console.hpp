#pragma once
#include <string>
#include "env.hpp"
#include "runtime/values.hpp"

inline std::unordered_map<std::string, Val> getConsole() {
    std::unordered_map<std::string, Val> mod = {
        { "println", std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
            for (Val arg : args) {
                std::cout << (arg->type == ValueType::Object ? arg->toConsole() : arg->toString()) << " ";
            }

            std::cout << std::endl;
            return std::make_shared<UndefinedVal>();
        }) },
        {"prompt", std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
            for (Val arg : args) {
                std::cout << arg->toString();
            }

            std::string input;
            std::getline(std::cin, input);

            return std::make_shared<StringVal>(input);
        })
    }};

    return mod;
}