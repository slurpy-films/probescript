#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>

namespace Config {

using namespace std;

enum RuntimeType {
    Normal,
    REPL,
    Exports,
};

struct Config {
    Config(RuntimeType type = Normal, string probeName = "Main") : type(type), probeName(probeName) {} 
    RuntimeType type;
    string probeName = "Main";
    std::unordered_map<std::string, filesystem::path> modules;
    bool importsDone = false;
};

}