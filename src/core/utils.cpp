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

const std::string ConsoleColors::RESET   = "\033[0m";
const std::string ConsoleColors::RED     = "\033[31m";
const std::string ConsoleColors::GREEN   = "\033[32m";
const std::string ConsoleColors::YELLOW  = "\033[33m";
const std::string ConsoleColors::BLUE    = "\033[34m";
const std::string ConsoleColors::MAGENTA = "\033[35m";
const std::string ConsoleColors::CYAN    = "\033[36m";
const std::string ConsoleColors::WHITE   = "\033[37m";
const std::string ConsoleColors::BOLD    = "\033[1m";

const std::string ConsoleColors::DARK_GRAY  = "\033[90m";
const std::string ConsoleColors::LIGHT_GRAY = "\033[37m";
const std::string ConsoleColors::GRAY       = "\033[2;37m";
const std::string ConsoleColors::DIM        = "\033[2m";

void logmsg(const std::string& msg)
{
    std::cout << msg;
}