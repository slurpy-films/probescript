#include "utils/split.hpp"
#include "utils/isNum.hpp"

bool isNum(const std::string& str)
{
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



std::vector<std::string> split(const std::string& str, char delimeter) 
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (getline(ss, item, delimeter)) {
        result.push_back(item);
    }

    return result;
}

std::vector<std::string> splitToChars(const std::string& str)
{
    std::vector<std::string> result;
    for (char c : str) {
        std::string s;
        s += c;
        result.push_back(s);
    }
    return result;
}