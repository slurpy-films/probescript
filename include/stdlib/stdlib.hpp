#pragma once
#include "fs.hpp"
#include "runtime/values.hpp"
#include <string>
#include <unordered_map>


std::unordered_map<std::string, ObjectVal*> stdlib = {
    {"Fs", new ObjectVal(filesystemModule)}
};