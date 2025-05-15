#pragma once
#include <string>
#include "runtime/values.hpp"

inline std::string Error(const std::string& m) {
    return ConsoleColors::RED + "[Error]: " + ConsoleColors::RESET + m + "\n";
}

inline std::string SyntaxError(const std::string& m) {
    return ConsoleColors::RED + "[SyntaxError]: " + ConsoleColors::RESET + m + "\n";
}

inline std::string TypeError(const std::string& m) {
    return ConsoleColors::RED + "[TypeError]: " + ConsoleColors::RESET + m + "\n";
}

inline std::string ArgumentError(const std::string& m) {
    return ConsoleColors::RED + "[ArgumentError]: " + ConsoleColors::RESET + m + "\n";
}

inline std::string ManualError(const std::string& m, const std::string& n) {
    return ConsoleColors::RED + "[" + n + "]: " + ConsoleColors::RESET + m + "\n";
}