#pragma once
#include <unordered_map>
#include <string>
#include <sstream>
#include <regex>
#include <algorithm>
#include "runtime/values.hpp"
#include "utils.hpp"
#include "types.hpp"

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

Val getValHttpModule();
TypePtr getTypeHttpModule();

#include "env.hpp"
#include "runtime/interpreter.hpp"

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


void startServer(const int port, std::function<void(Request, Response)> handler);

inline std::pair<std::string, std::string> parseMethodAndPath(const std::string& request)
{
    std::istringstream stream(request);
    std::string method, path;
    stream >> method >> path;
    return {method, path};
}

#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "threads.hpp"
#include <sstream>

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <sstream>
#include <threads.hpp>
#include <cstring>
#include <netdb.h>
#endif

Val sendReq(const std::string& method, std::string& url, std::shared_ptr<ObjectVal> conf, EnvPtr env);