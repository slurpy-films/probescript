#pragma once
#include <unordered_map>
#include <string>
#include <sstream>
#include <regex>
#include <algorithm>
#include "runtime/values.hpp"
#include "utils/split.hpp"

std::unordered_map<std::string, Val> getHttpModule();

#include "runtime/env.hpp"
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


void startServer(const int port, std::shared_ptr<std::unordered_map<std::string, std::unordered_map<std::string, Val>>> routes, Env* env);

inline std::pair<std::string, std::string> parseMethodAndPath(const std::string& request)
{
    std::istringstream stream(request);
    std::string method, path;
    stream >> method >> path;
    return {method, path};
}

#ifdef _WIN32

#pragma comment(lib, "Ws3_32.lib")

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

inline Val sendReq(const std::string& method, std::string& url, std::shared_ptr<ObjectVal> conf, Env* env)
{
    std::string headers;
    if (conf->hasProperty("headers") && conf->properties["headers"]->type == ValueType::Object)
    {
        for (auto& [key, val] : std::static_pointer_cast<ObjectVal>(conf->properties["headers"])->properties)
        {
            if (val->type == ValueType::String)
            {
                headers += key + ": " + std::static_pointer_cast<StringVal>(val)->string + "\r\n";
            }
        }
    }

    std::string body;
    if (conf->hasProperty("body") && conf->properties["body"]->type == ValueType::String) {
        body = std::static_pointer_cast<StringVal>(conf->properties["body"])->string;
    }

    std::regex urlRegex(R"(^(http?://)?([^:/]+)(:(\d+))?(/.*)?$)");
    std::smatch match;
    if (!std::regex_match(url, match, urlRegex)) {
        return env->throwErr("[HttpError]: Invalid URL format: " + url);
    }

    int port = 80;
    if (match[4].matched) {
        try {
            port = std::stoi(match[4]);
        } catch (...) {
            return env->throwErr("[HttpError]: Invalid port number in URL: " + match[4].str());
        }
    }

    std::string host = match[2];
    std::string path = match[5].matched ? match[5].str() : "/";
    std::string req = method + " " + path + " HTTP/1.1\r\n"
                      "Host: " + host + "\r\n"
                      "Connection: close\r\n"
                      "Content-length: " + std::to_string(body.length()) + "\r\n"
                      + headers + "\r\n"
                      + body;

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    struct hostent *server = gethostbyname(host.c_str());
    if (!server) {
        return env->throwErr("[HttpError]: Failed to resolve host: " + host);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return env->throwErr("[HttpError]: Socket creation failed");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = *(unsigned long*)server->h_addr;

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        return env->throwErr("[HttpError]: Connection to " + host + ":" + std::to_string(port) + " failed");
    }

    send(sock, req.c_str(), req.size(), 0);

    char buffer[4096];
    std::string res;
    int bytes;
    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = '\0';
        res += buffer;
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    size_t headerEnd = res.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return env->throwErr("[HttpError]: Malformed HTTP response");
    }

    std::string headerPart = res.substr(0, headerEnd);
    std::string bodyPart = res.substr(headerEnd + 4);

    std::istringstream stream(headerPart);
    std::string statusLine;
    std::getline(stream, statusLine);
    std::istringstream statusStream(statusLine);
    std::string httpVersion, statusCodeStr, statusText;
    statusStream >> httpVersion >> statusCodeStr;
    std::getline(statusStream, statusText);
    statusText = std::regex_replace(statusText, std::regex("^ +"), "");

    int statusCode = 0;
    try {
        statusCode = std::stoi(statusCodeStr);
    } catch (...) {
        return env->throwErr("[HttpError]: Invalid status code: " + statusCodeStr);
    }

    std::unordered_map<std::string, Val> props = {
        { "status", std::make_shared<NumberVal>(statusCode) },
        { "body", std::make_shared<NativeFnValue>([bodyPart](std::vector<Val>, Env*) -> Val {
            return std::make_shared<StringVal>(bodyPart);
        }) }
    };

    return std::make_shared<ObjectVal>(props);
}
