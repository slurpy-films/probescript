#pragma once
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include <unordered_map>
#include <string>
#include <random>

inline std::unordered_map<std::string, RuntimeVal*> createRandomModule() {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    return {
        {
            "randInt",
            new NativeFnValue([](std::vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
                if (args.size() < 2) {
                    std::cerr << "randInt expects two arguments";
                    exit(1);
                }

                std::uniform_int_distribution<> distrib(args[0]->toNum(), args[1]->toNum());
                return new NumberVal(distrib(gen));
            })
        },
        {
            "rand",
            new NativeFnValue([](std::vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
                std::uniform_real_distribution<double> distrib(0.0, 1.0);
                return new NumberVal(distrib(gen));
            })
        }
    };
}
