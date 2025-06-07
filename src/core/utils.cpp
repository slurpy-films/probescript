#include "utils.hpp"

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

std::vector<std::string> split(const std::string& str, const std::string& delimeter) 
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t end;

    while ((end = str.find(delimeter, start)) != std::string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + delimeter.length();
    }

    result.push_back(str.substr(start));
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