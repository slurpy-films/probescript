#pragma once
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include <unordered_map>
#include <string>
#include <random>

inline std::unordered_map<std::string, Val> createRandomModule() {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    return {
        {
            "randInt",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
                if (args.size() < 2) {
                    std::cerr << "randInt expects two arguments";
                    exit(1);
                }

                std::uniform_int_distribution<> distrib(args[0]->toNum(), args[1]->toNum());
                return std::make_shared<NumberVal>(distrib(gen));
            })
        },
        {
            "rand",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
                std::uniform_real_distribution<double> distrib(0.0, 1.0);
                return std::make_shared<NumberVal>(distrib(gen));
            })
        }
    };
}
