#pragma once
#include "fs.hpp"
#include "date.hpp"
#include "runtime/values.hpp"
#include <string>
#include <unordered_map>
#include "random.hpp"

std::unordered_map<std::string, ObjectVal*> stdlib = {
    {"Fs", new ObjectVal(filesystemModule)},
    {"Date", new ObjectVal(DateModule) },
    {"Random", new ObjectVal(createRandomModule()) },
};