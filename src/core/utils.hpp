#pragma once
#include <string>
#include <cctype>
#include <vector>
#include <sstream>
#include <iostream>

bool isNum(const std::string& str);

template <typename T>
inline T shift(std::vector<T>& vec) {
    if (vec.empty()) {
        std::cerr << "[EmptyVectorShiftError]: Cannot shift an empty vector";
        exit(1);
    }

    T first = vec.front();
    vec.erase(vec.begin());
    return first;
};

std::vector<std::string> split(const std::string& str, const std::string& delimeter);

std::vector<std::string> splitToChars(const std::string& str);