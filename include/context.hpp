#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include "runtime/values.hpp"

enum class RuntimeType {
    Normal,
    REPL,
    Exports,
};

struct Context {
    Context(RuntimeType type = RuntimeType::Normal, std::string probeName = "Main") : type(type), probeName(probeName) {} 
    RuntimeType type;
    std::string probeName = "Main";
    std::string filename = "REPL";
    std::string file;
    std::unordered_map<std::string, std::filesystem::path> modules;
    Val project;
};