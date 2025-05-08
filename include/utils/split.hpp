#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <sstream>


std::vector<std::string> split(const std::string& str, char delimeter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (getline(ss, item, delimeter)) {
        result.push_back(item);
    }

    return result;
}

std::vector<std::string> splitToChars(const std::string& str) {
    std::vector<std::string> result;
    for (char c : str) {
        std::string s;
        s += c;
        result.push_back(s);
    }
    return result;
}