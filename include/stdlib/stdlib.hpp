#pragma once
#include "fs.hpp"
#include "runtime/values.hpp"
#include <string>
#include <unordered_map>


unordered_map<string, ObjectVal*> stdlib = {
    {"Fs", new ObjectVal(filesystemModule)}
};