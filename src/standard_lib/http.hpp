#pragma once

#include <unordered_map>
#include <string>
#include <sstream>
#include <regex>
#include <algorithm>

#include "core/runtime/values.hpp"
#include "core/runtime/interpreter.hpp"
#include "core/utils.hpp"
#include "core/types.hpp"
#include "core/env.hpp"

namespace Probescript::Stdlib::Http
{

struct Request
{
    std::string method;
    std::string path;
    std::string raw;

    std::function<void(std::string)> ondata;
    std::function<void()> end;
    
    std::unordered_map<std::string, std::string> headers = {};
    std::unordered_map<std::string, std::string> cookies = {};
};

struct Response
{
    std::function<void(std::string, std::unordered_map<std::string, std::string>)> send;
};

Values::Val getValHttpModule();
Typechecker::TypePtr getTypeHttpModule();


inline std::string trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\r");
    const auto end = str.find_last_not_of(" \t\r");
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return str.substr(start, end - start + 1);
}

inline std::unordered_map<std::string, std::string> parseHeaders(const std::string& req)
{
    std::unordered_map<std::string, std::string> headers;
    std::istringstream stream(req);
    std::string line;

    std::getline(stream, line);

    while (std::getline(stream, line))
    {
        if (line == "\r" || line.empty()) break;

        size_t colon = line.find(':');
        if (colon != std::string::npos)
        {
            std::string key = trim(line.substr(0, colon));
            std::string value = trim(line.substr(colon + 1));
            headers[key] = value;
        }
    }

    return headers;
}

inline std::unordered_map<std::string, std::string> parseCookies(const std::string& c)
{
    std::unordered_map<std::string, std::string> cookies;
    for (const std::string& s : split(c, ";"))
    {
        std::string cookie = trim(s);
        auto parts = split(cookie, "=");
        if (parts.size() == 2) {
            std::string key = trim(parts[0]);
            std::string value = trim(parts[1]);
            cookies[key] = value;
        }
    }
    return cookies;
}


inline std::pair<std::string, std::string> parseMethodAndPath(const std::string& request)
{
    std::istringstream stream(request);
    std::string method, path;
    stream >> method >> path;
    return {method, path};
}

} // namespace Probescript::Stdlib::Http