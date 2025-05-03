#pragma once
#include <string>
#include <cctype>

bool isNum(const std::string& str) {
    if (str.empty()) return false;

    size_t i = 0;
    if (str[i] == '-') i++;

    bool hasDigits = false;
    bool hasDecimal = false;

    for (; i < str.size(); i++) {
        if (std::isdigit(str[i])) {
            hasDigits = true;
        } else if (str[i] == '.' && !hasDecimal) {
            hasDecimal = true;
        } else {
            return false;
        }
    }

    return hasDigits;
}
