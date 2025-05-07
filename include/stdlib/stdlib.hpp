#pragma once
#include "fs.hpp"
#include "date.hpp"
#include "runtime/values.hpp"
#include <string>
#include <unordered_map>
#include "random.hpp"

std::unordered_map<std::string, std::shared_ptr<ObjectVal>> getStdlib() {
    return {
        {"Fs", std::make_shared<ObjectVal>(getFilesystemModule())},
        {"Date", std::make_shared<ObjectVal>(getDateModule()) },
        {"Random", std::make_shared<ObjectVal>(createRandomModule()) }
    };
};