#pragma once
#include <string>
#include <cctype>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>

class ThrowException : public std::exception
{
public:
    ThrowException(const std::string& m);
    const char* what() const noexcept;
private:
    std::string m_msg;
};

bool isNum(const std::string& str);

template <typename T>
inline T shift(std::vector<T>& vec) {
    if (vec.empty()) {
        throw std::runtime_error("[EmptyVectorShiftError]: Cannot shift an empty vector");
    }

    T first = vec.front();
    vec.erase(vec.begin());
    return first;
};

std::vector<std::string> split(const std::string& str, const std::string& delimeter);
std::vector<std::string> splitToChars(const std::string& str);

namespace ConsoleColors {
    extern const std::string RESET;
    extern const std::string RED;
    extern const std::string GREEN ;
    extern const std::string YELLOW;
    extern const std::string BLUE;
    extern const std::string MAGENTA;
    extern const std::string CYAN;
    extern const std::string WHITE;
    extern const std::string BOLD;
    
    extern const std::string DARK_GRAY;
    extern const std::string LIGHT_GRAY;
    extern const std::string GRAY;
    extern const std::string DIM;
}

void logmsg(const std::string& msg);
#define LOG(msg) logmsg(msg);