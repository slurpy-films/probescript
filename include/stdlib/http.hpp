#pragma once
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "runtime/eval/call.hpp"
#include "sstream"

#include <unordered_map>
#include <string>
#include <sstream>
#include <algorithm>

std::string trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\r");
    const auto end = str.find_last_not_of(" \t\r");
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return str.substr(start, end - start + 1);
}

std::unordered_map<std::string, std::string> parseHeaders(const std::string& req) {
    std::unordered_map<std::string, std::string> headers;
    std::istringstream stream(req);
    std::string line;

    std::getline(stream, line);

    while (std::getline(stream, line)) {
        if (line == "\r" || line.empty()) break;

        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = trim(line.substr(0, colon));
            std::string value = trim(line.substr(colon + 1));
            headers[key] = value;
        }
    }

    return headers;
}


#ifdef _WIN32

#pragma comment(lib, "Ws3_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "threads.hpp"
#include <sstream>

std::pair<std::string, std::string> parseMethodAndPath(const std::string& request) {
    std::istringstream stream(request);
    std::string method, path;
    stream >> method >> path;
    return {method, path};
}

void startServer(const int port, std::shared_ptr<std::unordered_map<std::string, std::unordered_map<std::string, Val>>> routes, Env* env) {
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    listen(serverSocket, SOMAXCONN);

    while (true) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);

        if (clientSocket == INVALID_SOCKET) continue;

        std::thread([clientSocket, routes, env]() {
            char buffer[4096];
            int bytesRecieved = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRecieved <= 0) {
                closesocket(clientSocket);
                return;
            }

            std::string request(buffer, bytesRecieved);
            auto [method, path] = parseMethodAndPath(request);

            auto routeMap = routes->find(method);
            if (routeMap != routes->end()) {
                auto handlerIt = routeMap->second.find(path);
                if (handlerIt != routeMap->second.end()) {
                    Val handler = handlerIt->second;

                    std::shared_ptr<ObjectVal> req = std::make_shared<ObjectVal>();
                    req->properties["path"] = std::make_shared<StringVal>(path);
                    req->properties["method"] = std::make_shared<StringVal>(method);
                    req->properties["raw"] = std::make_shared<NativeFnValue>([request](std::vector<Val> args, Env* env) -> Val {
                        return std::make_shared<StringVal>(request);
                    });
                    auto headersMap = std::make_shared<ObjectVal>();
                    for (const auto& [key, value] : parseHeaders(request)) {
                        headersMap->properties[key] = std::make_shared<StringVal>(value);
                    }
                    req->properties["headers"] = headersMap;

                    std::shared_ptr<ObjectVal> res = std::make_shared<ObjectVal>();
                    res->properties["send"] = std::make_shared<NativeFnValue>([clientSocket](std::vector<Val> args, Env* _) -> Val {
                        if (args.empty()) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                 << "Content-Type: text/plain\r\n"
                                 << "Content-Length: " << body.size() << "\r\n"
                                 << "Connection: close\r\n\r\n"
                                 << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);
                        closesocket(clientSocket);
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["html"] = std::make_shared<NativeFnValue>([clientSocket](std::vector<Val> args, Env* _) -> Val {
                        if (args.empty() || args[0]->type != ValueType::String) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                 << "Content-Type: text/html\r\n"
                                 << "Content-Length: " << body.size() << "\r\n"
                                 << "Connection: close\r\n\r\n"
                                 << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);
                        closesocket(clientSocket);
                        return std::make_shared<UndefinedVal>();
                    });

                    evalCallWithFnVal(handler, { req, res }, env);
                } else {
                    const char* notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
                    send(clientSocket, notFound, strlen(notFound), 0);
                    closesocket(clientSocket);
                }
            }
        }).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
}

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <sstream>
#include <threads.hpp>
#include <cstring>

std::pair<std::string, std::string> parseMethodAndPath(const std::string& request) {
    std::istringstream stream(request);
    std::string method, path;
    stream >> method >> path;
    return {method, path};
}

void startServer(const int port, std::shared_ptr<std::unordered_map<std::string, std::unordered_map<std::string, Val>>> routes, Env* env) {
    int serverSocket;
    struct sockaddr_in serverAddr;


    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed";
        close(serverSocket);
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        close(serverSocket);
        return;
    }

    listen(serverSocket, SOMAXCONN);

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientLen < 0) continue;

        std::thread([clientSocket, routes, env]() {
            char buffer[4096];
            int bytesRecieved = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRecieved <= 0) {
                close(clientSocket);
                return;
            }

            std::string request(buffer, bytesRecieved);
            auto [method, path] = parseMethodAndPath(request);

            auto routeMap = routes->find(method);
            if (routeMap != routes->end()) {
                auto handlerIt = routeMap->second.find(path);
                if (handlerIt != routeMap->second.end()) {
                    Val handler = handlerIt->second;


                    std::shared_ptr<ObjectVal> req = std::make_shared<ObjectVal>();
                    req->properties["path"] = std::make_shared<StringVal>(path);
                    req->properties["method"] = std::make_shared<StringVal>(method);
                    req->properties["raw"] = std::make_shared<NativeFnValue>([request](std::vector<Val> args, Env* env) -> Val {
                        return std::make_shared<StringVal>(request);
                    });
                    std::shared_ptr<ObjectVal> headers = std::make_shared<ObjectVal>();
                    for (auto& [key, value] : parseHeaders(request)) {
                        headers->properties[key] = std::make_shared<StringVal>(value);
                    }
                    req->properties["headers"] = headers;

                    std::shared_ptr<ObjectVal> res = std::make_shared<ObjectVal>();
                    res->properties["send"] = std::make_shared<NativeFnValue>([clientSocket](std::vector<Val> args, Env* _) -> Val {
                        if (args.empty()) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                << "Content-Type: text/plain\r\n"
                                << "Content-Length: " << body.size() << "\r\n"
                                << "Connection: close\r\n\r\n"
                                << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);
                        close(clientSocket);
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["html"] = std::make_shared<NativeFnValue>([clientSocket](std::vector<Val> args, Env* _) -> Val {
                        if (args.empty() || args[0]->type != ValueType::String) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                << "Content-Type: text/html\r\n"
                                << "Content-Length: " << body.size() << "\r\n"
                                << "Connection: close\r\n\r\n"
                                << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);
                        close(clientSocket);
                        return std::make_shared<UndefinedVal>();
                    });

                    evalCallWithFnVal(handler, { req, res }, env);
                } else {
                    const char* notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
                    send(clientSocket, notFound, strlen(notFound), 0);
                    close(clientSocket);
                }
            }
        }).detach();
    }
}

#endif

std::unordered_map<std::string, Val> getHttpModule() {
    Env* env = new Env();
    return {
        {
            "Server",
            std::make_shared<NativeClassVal>([](std::vector<Val> args, Env* env) -> Val {

                std::shared_ptr<ObjectVal> t = std::make_shared<ObjectVal>();

                auto routeHandlers = std::make_shared<std::unordered_map<std::string, std::unordered_map<std::string, Val>>>();

                t->properties["get"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        std::cerr << "Usage: get('/path', handlerFn)\n";
                        exit(1);
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["GET"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });
                
                t->properties["post"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        std::cerr << "Usage: post('/path', handlerFn)\n";
                        exit(1);
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["POST"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["put"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        std::cerr << "Usage: put('/path', handlerFn)\n";
                        exit(1);
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["PUT"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["delete"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        std::cerr << "Usage: delete('/path', handlerFn)\n";
                        exit(1);
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["DELETE"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["patch"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        std::cerr << "Usage: patch('/path', handlerFn)\n";
                        exit(1);
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["PATCH"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["head"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        std::cerr << "Usage: head('/path', handlerFn)\n";
                        exit(1);
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["GET"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });
                
                t->properties["listen"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (!args[0]) {
                        std::cerr << "Argument 0 is null!";
                        exit(1);
                    }
                    if (args[0]->type != ValueType::Number) {
                        std::cerr << "Expected argument 1 to be of type number";
                        exit(1);
                    }                    
                    
                    const int port = std::static_pointer_cast<NumberVal>(args[0])->number;

                    std::thread serverThread(startServer, port, routeHandlers, new Env(env));
                    registerThread(std::move(serverThread));

                    if (args[1]->type == ValueType::Function) {
                        return evalCallWithFnVal(args[1], std::vector<Val>(), env);
                    }

                    return std::make_shared<UndefinedVal>();
                });

                return t;
            })
        },
    };
};