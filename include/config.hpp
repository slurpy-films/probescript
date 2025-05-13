#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>

namespace Config {


enum RuntimeType {
    Normal,
    REPL,
    Exports,
};

struct Config {
    Config(RuntimeType type = Normal, std::string probeName = "Main") : type(type), probeName(probeName) {} 
    RuntimeType type;
    std::string probeName = "Main";
    std::unordered_map<std::string, std::filesystem::path> modules;
    Val project;
};

}