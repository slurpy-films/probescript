#pragma once
#include <chrono>
#include <string>
#include "runtime/values.hpp"
#include <unordered_map>
#include <algorithm>

std::unordered_map<std::string, Val> getDateModule() {
    return {
        {"stamp", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto duration = now.time_since_epoch();

        std::string unit = "sec";
        if (!args.empty() && args[0]->type == ValueType::String) {
            unit = std::static_pointer_cast<StringVal>(args[0])->value;
            std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
        }

        long long result;

        if (unit == "milli") {
            result = duration_cast<milliseconds>(duration).count();
        } else if (unit == "sec") {
            result = duration_cast<seconds>(duration).count();
        } else if (unit == "min") {
            result = duration_cast<minutes>(duration).count();
        } else if (unit == "hour") {
            result = duration_cast<hours>(duration).count();
        } else {
            return std::make_shared<StringVal>("Invalid time unit: " + unit);
        }

        return std::make_shared<NumberVal>(std::to_string(result));
    })}};
};
