#pragma once
#include <string>
#include "runtime/values.hpp"
#include "frontend/lexer.hpp"
#include "context.hpp"
#include "utils.hpp"

inline std::string Error(const std::string& m) {
    return ConsoleColors::RED + "[Error]: " + ConsoleColors::RESET + m + "\n";
}

inline std::string SyntaxError(const std::string& m, Lexer::Token tk = Lexer::Token(), std::shared_ptr<Context> ctx = std::make_shared<Context>()) {
    std::string line = split(ctx->file, "\n")[tk.line - 1];
    if (line == "")
        return ConsoleColors::RED + "[SyntaxError]: " + ConsoleColors::RESET + m + "\n";
    else
    {        
        return ConsoleColors::RED + "[SyntaxError]: " + ConsoleColors::RESET + m + "\n\n" + "At " + ctx->filename + ":" + std::to_string(tk.line) + ":" + std::to_string(tk.col) + "\n" + line + "\n";
    }
}

inline std::string TypeError(const std::string& m, Lexer::Token tk = Lexer::Token(), std::shared_ptr<Context> ctx = std::make_shared<Context>()) {
    std::string line = split(ctx->file, "\n")[tk.line - 1];
    if (line == "")
        return ConsoleColors::RED + "[TypeError]: " + ConsoleColors::RESET + m + "\n";
    else
    {        
        return ConsoleColors::RED + "[TypeError]: " + ConsoleColors::RESET + m + "\n\n" + "At " + ctx->filename + ":" + std::to_string(tk.line) + ":" + std::to_string(tk.col) + "\n" + line + "\n";
    }
}

inline std::string ArgumentError(const std::string& m) {
    return ConsoleColors::RED + "[ArgumentError]: " + ConsoleColors::RESET + m + "\n";
}

inline std::string ManualError(const std::string& m, const std::string& n) {
    return ConsoleColors::RED + "[" + n + "]: " + ConsoleColors::RESET + m + "\n";
}

inline std::string ManualError(const std::string& m, const std::string& n, Lexer::Token tk) {
    std::string line = split(tk.ctx->file, "\n")[tk.line - 1];
    if (line.empty())
        return ConsoleColors::RED + "[" + n + "]: " + ConsoleColors::RESET + m + "\n";
    else
    {        
        return ConsoleColors::RED + "[" + n + "]: " + ConsoleColors::RESET + m + "\n\n" + "At " + tk.ctx->filename + ":" + std::to_string(tk.line) + ":" + std::to_string(tk.col) + "\n" + line + "\n";
    }
}