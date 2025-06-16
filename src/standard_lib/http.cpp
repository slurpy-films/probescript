#include "http.hpp"

void startServer(const int port, std::shared_ptr<std::unordered_map<std::string, std::unordered_map<std::string, Val>>> routes, EnvPtr env)
{
    struct sockaddr_in serverAddr;
#ifdef _WIN32
    WSADATA wsaData;
    SOCKET serverSocket;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return;
    }
#else
    int serverSocket;
#endif

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return;
    }
#else
    if (serverSocket < 0)
    {
        std::cerr << "Socket creation failed\n";
        close(serverSocket);
        return;
    }
#endif

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

#ifdef _WIN32
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }
#else
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Bind failed\n";
        close(serverSocket);
        return;
    }
#endif

    listen(serverSocket, SOMAXCONN);

    while (true)
    {
        struct sockaddr_in clientAddr;

#ifdef _WIN32
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) continue;
#else
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientLen < 0) continue;
#endif

        std::thread([clientSocket, routes, env]()
        {
            char buffer[4096];
            int bytesRecieved = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRecieved <= 0)
            {
#ifdef _WIN32
                closesocket(clientSocket);
#else
                close(clientSocket);
#endif
                return;
            }

            std::string request(buffer, bytesRecieved);
            auto [method, path] = parseMethodAndPath(request);

            auto routeMap = routes->find(method);
            if (routeMap != routes->end())
            {
                auto handlerIt = routeMap->second.find(path);
                if (handlerIt != routeMap->second.end()) {
                    Val handler = handlerIt->second;

                    std::shared_ptr<ObjectVal> req = std::make_shared<ObjectVal>();
                    req->properties["path"] = std::make_shared<StringVal>(path);
                    req->properties["method"] = std::make_shared<StringVal>(method);
                    req->properties["raw"] = std::make_shared<NativeFnValue>([request](std::vector<Val> args, EnvPtr env) -> Val
                    {
                        return std::make_shared<StringVal>(request);
                    });

                    std::shared_ptr<ObjectVal> headersMap = std::make_shared<ObjectVal>();
                    for (const auto& [key, value] : parseHeaders(request))
                    {
                        headersMap->properties[key] = std::make_shared<StringVal>(value);
                    }

                    req->properties["headers"] = headersMap;
                    if (headersMap->hasProperty("Cookie")) {
                        std::unordered_map<std::string, std::string> raw = parseCookies(headersMap->properties["Cookie"]->toString());
                        auto cookies = std::make_shared<ObjectVal>();

                        for (auto& pair : raw)
                        {
                            cookies->properties[pair.first] = std::make_shared<StringVal>(pair.second);
                        }

                        req->properties["cookies"] = cookies;
                    }
                    else req->properties["cookies"] = std::make_shared<ObjectVal>();

                    auto resheaders = std::make_shared<std::string>();
                    auto contype = std::make_shared<std::string>("text/plain");

                    std::shared_ptr<ObjectVal> res = std::make_shared<ObjectVal>();
                    res->properties["content_type"] = std::make_shared<NativeFnValue>([contype](std::vector<Val> args, EnvPtr env) -> Val
                    {
                        if (args.empty()) return env->throwErr(ArgumentError("Usage: res.contentType(type)"));
                        (*contype) = args[0]->toString();
                        
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["send"] = std::make_shared<NativeFnValue>([clientSocket, resheaders, contype](std::vector<Val> args, EnvPtr _) -> Val
                    {
                        if (args.empty()) return std::make_shared<UndefinedVal>();
                        std::string body = args[0]->toString();
                        std::ostringstream response;
                        response << "HTTP/1.1 200\r\n"
                                << "Content-Type: " << *contype << "\r\n"
                                << "Content-Length: " << body.size() << "\r\n"
                                << "Connection: close\r\n"
                                << *resheaders << "\r\n"
                                << body;

                        std::string resStr = response.str();
                        send(clientSocket, resStr.c_str(), resStr.size(), 0);

#ifdef _WIN32
                        closesocket(clientSocket);
#else
                        close(clientSocket);
#endif
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["html"] = std::make_shared<NativeFnValue>([clientSocket, resheaders](std::vector<Val> args, EnvPtr _) -> Val 
                    {
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
#ifdef _WIN32
                        closesocket(clientSocket);
#else
                        close(clientSocket);
#endif
                        return std::make_shared<UndefinedVal>();
                    });

                    res->properties["cookie"] = std::make_shared<NativeFnValue>([resheaders](std::vector<Val> args, EnvPtr env) -> Val
                    {
                        if (args.size() < 2) return env->throwErr(ArgumentError("Usage: cookie(name, value)"));

                        *resheaders += "Set-Cookie: " + args[0]->toString() + "=" + args[1]->toString() + "\r\n";

                        return std::make_shared<UndefinedVal>();
                    });

                    evalCallWithFnVal(handler, { req, res }, env);
                }
                else
                {
                    const char* notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
                    send(clientSocket, notFound, strlen(notFound), 0);
#ifdef _WIN32
                closesocket(clientSocket);
#else
                close(clientSocket);
#endif
                }
            }
        }).detach();
    }

#ifdef _WIN32
    closesocket(serverSocket);
    WSACleanup();
#else
    close(serverSocket);
#endif
}

Val sendReq(const std::string& method, std::string& url, std::shared_ptr<ObjectVal> conf, EnvPtr env)
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
        { "body", std::make_shared<NativeFnValue>([bodyPart](std::vector<Val>, EnvPtr) -> Val {
            return std::make_shared<StringVal>(bodyPart);
        }) }
    };

    return std::make_shared<ObjectVal>(props);
}

Val getValHttpModule()
{
    return std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>({
        {
            "Server",
            std::make_shared<NativeClassVal>([](std::vector<Val> args, EnvPtr env) -> Val {

                std::shared_ptr<ObjectVal> t = std::make_shared<ObjectVal>();

                auto routeHandlers = std::make_shared<std::unordered_map<std::string, std::unordered_map<std::string, Val>>>();

                t->properties["get"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, EnvPtr env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: get('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["GET"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });
                
                t->properties["post"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, EnvPtr env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: post('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["POST"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["put"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, EnvPtr env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: put('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["PUT"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["delete"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, EnvPtr env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: delete('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["DELETE"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["patch"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, EnvPtr env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: patch('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["PATCH"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });

                t->properties["head"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, EnvPtr env) -> Val {
                    if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Function) {
                        return env->throwErr(ArgumentError("Usage: head('/path', handlerFn)"));
                    }
                    std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                    (*routeHandlers)["GET"][path] = args[1];
                    return std::make_shared<UndefinedVal>();
                });
                
                t->properties["listen"] = std::make_shared<NativeFnValue>([routeHandlers](std::vector<Val> args, EnvPtr env) -> Val {
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
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) return env->throwErr(ArgumentError("Usage: http.get(\"http://example.com\", { body: \"body\", headers: {}})"));

                return sendReq("GET", std::static_pointer_cast<StringVal>(args[0])->string, std::static_pointer_cast<ObjectVal>(args[1]), env);
            })
        },
        {
            "post",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) return env->throwErr(ArgumentError("Usage: http.post(\"http://example.com\", { body: \"body\", headers: {}})"));

                return sendReq("POST", std::static_pointer_cast<StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == ValueType::Object) ? std::static_pointer_cast<ObjectVal>(args[1]) : std::make_shared<ObjectVal>()), env);
            })
        }
    }));
};

TypePtr getTypeHttpModule()
{
    return std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
        {
            "Server",
            std::make_shared<Type>(TypeKind::Class, "native class", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
                {
                    "listen",
                    std::make_shared<Type>(TypeKind::Function, "native method", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "port", new IdentifierType("num")) })))
                },
                {
                    "get",
                    std::make_shared<Type>(TypeKind::Function, "native method", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                },
                {
                    "post",
                    std::make_shared<Type>(TypeKind::Function, "native method", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                },
                {
                    "put",
                    std::make_shared<Type>(TypeKind::Function, "native method", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                },
                {
                    "delete",
                    std::make_shared<Type>(TypeKind::Function, "native method", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                },
                {
                    "patch",
                    std::make_shared<Type>(TypeKind::Function, "native method", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                },
                {
                    "head",
                    std::make_shared<Type>(TypeKind::Function, "native method", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                }
            })), "httpServer")
        },
        {
            "get",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        },
        {
            "post",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        }
    })));
};