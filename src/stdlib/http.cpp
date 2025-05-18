#include "stdlib/http.hpp"

#ifdef _WIN32
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
                    std::shared_ptr<ObjectVal> headersMap = std::make_shared<ObjectVal>();
                    for (const auto& [key, value] : parseHeaders(request)) {
                        headersMap->properties[key] = std::make_shared<StringVal>(value);
                    }
                    req->properties["headers"] = headersMap;    
                    if (headersMap->hasProperty("Cookie")) {
                        std::unordered_map<std::string, std::string> raw = parseCookies(headersMap->properties["Cookie"]->toString());
                        auto cookies = std::make_shared<ObjectVal>();

                        for (auto& pair : raw) {
                            cookies->properties[pair.first] = std::make_shared<StringVal>(pair.second);
                        }

                        req->properties["cookies"] = cookies;
                    } else req->properties["cookies"] = std::make_shared<ObjectVal>();

                    auto resheaders = std::make_shared<std::string>();

                    std::shared_ptr<ObjectVal> res = std::make_shared<ObjectVal>();
                    res->properties["send"] = std::make_shared<NativeFnValue>([clientSocket, resheaders](std::vector<Val> args, Env* _) -> Val {
                        if (args.empty()) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                 << "Content-Type: text/plain\r\n"
                                 << "Content-Length: " << body.size() << "\r\n"
                                 << "Connection: close\r\n"
                                 << *resheaders << "\r\n"
                                 << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);
                        closesocket(clientSocket);
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["html"] = std::make_shared<NativeFnValue>([clientSocket, resheaders](std::vector<Val> args, Env* _) -> Val {
                        if (args.empty() || args[0]->type != ValueType::String) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                 << "Content-Type: text/html\r\n"
                                 << "Content-Length: " << body.size() << "\r\n"
                                 << "Connection: close\r\n"
                                 << resheaders << "\r\n"
                                 << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);
                        closesocket(clientSocket);
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["cookie"] = std::make_shared<NativeFnValue>([resheaders](std::vector<Val> args, Env* env) -> Val {
                        if (args.size() < 2) return env->throwErr(ArgumentError("Usage: cookie(name, value)"));

                        *resheaders += "Set-Cookie: " + args[0]->toString() + "=" + args[1]->toString() + "\r\n";

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
                    if (headersMap->hasProperty("Cookie")) {
                        std::unordered_map<std::string, std::string> raw = parseCookies(headersMap->properties["Cookie"]->toString());
                        auto cookies = std::make_shared<ObjectVal>();

                        for (auto& pair : raw) {
                            cookies->properties[pair.first] = std::make_shared<StringVal>(pair.second);
                        }

                        req->properties["cookies"] = cookies;
                    } else req->properties["cookies"] = std::make_shared<ObjectVal>();

                    auto resheaders = std::make_shared<std::string>();

                    std::shared_ptr<ObjectVal> res = std::make_shared<ObjectVal>();
                    res->properties["send"] = std::make_shared<NativeFnValue>([clientSocket, resheaders](std::vector<Val> args, Env* _) -> Val {
                        if (args.empty()) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                << "Content-Type: text/plain\r\n"
                                << "Content-Length: " << body.size() << "\r\n"
                                << "Connection: close\r\n"
                                << *resheaders << "\r\n"
                                << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);
                        close(clientSocket);
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["html"] = std::make_shared<NativeFnValue>([clientSocket, resheaders](std::vector<Val> args, Env* _) -> Val {
                        if (args.empty() || args[0]->type != ValueType::String) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                << "Content-Type: text/html\r\n"
                                << "Content-Length: " << body.size() << "\r\n"
                                << "Connection: close\r\n"
                                << *resheaders << "\r\n"
                                << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);
                        close(clientSocket);
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["cookie"] = std::make_shared<NativeFnValue>([resheaders](std::vector<Val> args, Env* env) -> Val {
                        if (args.size() < 2) return env->throwErr(ArgumentError("Usage: cookie(name, value)"));

                        *resheaders += "Set-Cookie: " + args[0]->toString() + "=" + args[1]->toString() + "\r\n";

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
                        return env->throwErr(ArgumentError("Usage: get('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["GET"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });
                
                t->properties["post"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: post('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["POST"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["put"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: put('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["PUT"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["delete"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: delete('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["DELETE"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["patch"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: patch('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["PATCH"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["head"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: head('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["GET"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });
                
                t->properties["listen"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, Env* env) -> Val {
                    if (args.empty()) {
                        return env->throwErr(ArgumentError("Usage: listen(port, callback (optional))"));
                    }
                    if (args[0]->type != ValueType::Number) {
                        return env->throwErr(ArgumentError("Usage: listen(port, callback (optional))"));
                    }
                    
                    const int port = std::static_pointer_cast<NumberVal>(args[0])->number;

                    std::thread serverThread([port, routeHandlers, env]() {
                        try {
                            startServer(port, routeHandlers, env);
                        } catch (...) {
                            env->throwErr(ManualError("Exception in server", "HttpError"));
                        }
                    });

                    threadManager.registerThread(std::move(serverThread));

                    if (args.size() >= 2 && args[1]->type == ValueType::Function) {
                        return evalCallWithFnVal(args[1], std::vector<Val>(), env);
                    }

                    return std::make_shared<UndefinedVal>();
                });

                return t;
            })
        },
        {
            "get",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) return env->throwErr(ArgumentError("Usage: http.get(\"http://example.com\", { body: \"body\", headers: {}})"));

                return sendReq("GET", std::static_pointer_cast<StringVal>(args[0])->string, std::static_pointer_cast<ObjectVal>(args[1]), env);
            })
        },
        {
            "post",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) return env->throwErr(ArgumentError("Usage: http.post(\"http://example.com\", { body: \"body\", headers: {}})"));

                return sendReq("POST", std::static_pointer_cast<StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == ValueType::Object) ? std::static_pointer_cast<ObjectVal>(args[1]) : std::make_shared<ObjectVal>()), env);
            })
        }
    };
};