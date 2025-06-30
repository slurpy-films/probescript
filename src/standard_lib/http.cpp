#include "http.hpp"

void startServer(const int port, std::function<void(std::shared_ptr<Request>, std::shared_ptr<Response>)> handler)
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
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error(CustomError("Bind failed", "HttpError"));
    }
#else
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        close(serverSocket);
        throw std::runtime_error(CustomError("Bind failed", "HttpError"));
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
        if (clientSocket < 0) continue;
#endif

        std::thread([clientSocket, handler]()
        {
            std::string request = "";
            bool headersParsed = false;
            size_t contentLength = 0;
            size_t headerEnd = 0;
            size_t bodyReceived = 0;
            std::string bufferedBody = ""; // Buffer for body data received before handler sets ondata
            
            auto req = std::make_shared<Request>();
            auto res = std::make_shared<Response>();
            
            res->send = [clientSocket](std::string body, std::unordered_map<std::string, std::string> headers) -> void
            {
                std::ostringstream response;
                response << "HTTP/1.1 200\r\n"
                        << "Content-Length: " << body.size() << "\r\n"
                        << "Connection: close\r\n";

                for (const auto& [key, val] : headers)
                {
                    response << key << ": " << val << "\r\n";
                }

                response << "\r\n" << body;

                std::string resStr = response.str();
                send(clientSocket, resStr.c_str(), resStr.size(), 0);

#ifdef _WIN32
                closesocket(clientSocket);
#else
                close(clientSocket);
#endif
            };
            
            while (true)
            {
                char buffer[4096];
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesReceived <= 0) break;
                
                if (!headersParsed)
                {
                    request.append(buffer, bytesReceived);
                    headerEnd = request.find("\r\n\r\n");
                    if (headerEnd != std::string::npos)
                    {
                        headersParsed = true;
                        std::string headerPart = request.substr(0, headerEnd);
                        
                        auto [method, path] = parseMethodAndPath(headerPart);
                        req->path = path;
                        req->method = method;
                        req->raw = headerPart;
                        
                        std::unordered_map<std::string, std::string> headersMap = {};
                        for (const auto& [key, value] : parseHeaders(headerPart))
                        {
                            headersMap[key] = value;
                        }
                        
                        req->headers = headersMap;
                        if (headersMap.find("Cookie") != headersMap.end())
                        {
                            std::unordered_map<std::string, std::string> cookies = parseCookies(headersMap["Cookie"]);
                            req->cookies = cookies;
                        }
                        
                        if (headersMap.find("Content-Length") != headersMap.end())
                        {
                            contentLength = std::stoul(headersMap["Content-Length"]);
                        }
                        
                        handler(req, res);
                        
                        if (request.length() > headerEnd + 4)
                        {
                            std::string bodyPart = request.substr(headerEnd + 4);
                            bufferedBody += bodyPart;
                            bodyReceived += bodyPart.length();
                        }
                        
                        if (!bufferedBody.empty() && req->ondata)
                        {
                            req->ondata(bufferedBody);
                        }
                    }
                }
                else
                {
                    std::string bodyPart(buffer, bytesReceived);
                    
                    if (req->ondata)
                    {
                        req->ondata(bodyPart);
                    }
                    bodyReceived += bytesReceived;
                }
                
                if (headersParsed && bodyReceived >= contentLength) break;
                if (headersParsed && contentLength == 0) break;
            }
            
            if (req->end) req->end();

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
        throw ThrowException("[HttpError]: Invalid URL format: " + url);
    }

    int port = 80;
    if (match[4].matched) {
        try {
            port = std::stoi(match[4]);
        } catch (...) {
            throw ThrowException("[HttpError]: Invalid port number in URL: " + match[4].str());
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
        throw ThrowException("[HttpError]: Failed to resolve host: " + host);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw ThrowException("[HttpError]: Socket creation failed");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = *(unsigned long*)server->h_addr;

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw ThrowException("[HttpError]: Connection to " + host + ":" + std::to_string(port) + " failed");
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
        throw ThrowException("[HttpError]: Malformed HTTP response");
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
        throw ThrowException("[HttpError]: Invalid status code: " + statusCodeStr);
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
            "Serve",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (
                    args.empty()
                    || args[0]->type != ValueType::Object
                    || args[0]->properties.find("port") == args[0]->properties.end()
                    || args[0]->properties.find("handler") == args[0]->properties.end()
                    || args[0]->properties["port"]->type != ValueType::Number
                    || args[0]->properties["handler"]->type != ValueType::Function
                ) throw ThrowException(ArgumentError("Usage: http.Serve({ port: number, handler: function })"));

                startServer(
                    std::static_pointer_cast<NumberVal>(args[0]->properties["port"])->number,
                    [args, env](std::shared_ptr<Request> request, std::shared_ptr<Response> response) -> void
                    {
                        std::shared_ptr<ObjectVal> req = std::make_shared<ObjectVal>();
                        std::shared_ptr<ObjectVal> res = std::make_shared<ObjectVal>();

                        req->properties["path"] = std::make_shared<StringVal>(request->path);
                        req->properties["method"] = std::make_shared<StringVal>(request->method);
                        req->properties["headers"] = std::make_shared<ObjectVal>();
                        req->properties["cookies"] = std::make_shared<ObjectVal>();

                        req->properties["ondata"] = std::make_shared<NativeFnValue>([request](std::vector<Val> args, EnvPtr env) -> Val
                        {
                            if (args.empty() || args[0]->type != ValueType::Function) 
                                throw ThrowException(ArgumentError("Usage: req.ondata(callback: function)"));
                            
                            request->ondata = std::function<void(std::string)>([args, env](std::string data)
                            {
                                evalCallWithFnVal(args[0], { std::make_shared<StringVal>(data) }, env);
                            });

                            return std::make_shared<UndefinedVal>();
                        });

                        req->properties["end"] = std::make_shared<NativeFnValue>([request](std::vector<Val> args, EnvPtr _env) -> Val
                        {
                            if (args.empty() || args[0]->type != ValueType::Function) 
                                throw ThrowException(ArgumentError("Usage: req.end(callback: function)"));
                            
                            request->end = std::function<void()>([args, _env]()
                            {
                                evalCallWithFnVal(args[0], {}, _env);
                            });

                            return std::make_shared<UndefinedVal>();
                        });

                        for (const auto& [key, val] : request->headers)
                            req->properties["headers"]->properties[key] = std::make_shared<StringVal>(val);

                        for (const auto& [key, val] : request->cookies)
                            req->properties["cookies"]->properties[key] = std::make_shared<StringVal>(val);

                        req->properties["raw"] =
                        std::make_shared<NativeFnValue>([request](std::vector<Val> _args, EnvPtr _env) -> Val
                        {
                            return std::make_shared<StringVal>(request->raw);
                        });

                        auto resheaders = std::make_shared<std::unordered_map<std::string, std::string>>();
                        (*resheaders)["Content-Type"] = "text/plain";

                        res->properties["content_type"] = std::make_shared<NativeFnValue>([resheaders](std::vector<Val> args, EnvPtr env) -> Val
                        {
                            if (args.empty()) throw ThrowException(ArgumentError("Usage: res.content_type(type: str)"));

                            (*resheaders)["Content-Type"] = args[0]->toString();

                            return std::make_shared<UndefinedVal>();
                        });

                        res->properties["send"] = std::make_shared<NativeFnValue>([resheaders, response](std::vector<Val> args, EnvPtr env) -> Val
                        {
                            if (args.empty()) throw ThrowException(ArgumentError("Usage: res.send(value: str)"));

                            response->send(args[0]->toString(), (*resheaders));

                            return std::make_shared<UndefinedVal>();
                        });

                        res->properties["html"] = std::make_shared<NativeFnValue>([resheaders, response](std::vector<Val> args, EnvPtr env) -> Val
                        {
                            if (args.empty()) throw ThrowException(ArgumentError("Usage: res.html(html: str)"));

                            (*resheaders)["Content-Type"] = "text/html";
                            response->send(args[0]->toString(), (*resheaders));

                            return std::make_shared<UndefinedVal>();
                        });

                        res->properties["json"] = std::make_shared<NativeFnValue>([resheaders, response](std::vector<Val> args, EnvPtr env) -> Val
                        {
                            if (args.empty() || args[0]->type != ValueType::Object) throw ThrowException(ArgumentError("Usage: res.html(object: object)"));

                            (*resheaders)["Content-Type"] = "application/json";
                            response->send(args[0]->toString(), (*resheaders));

                            return std::make_shared<UndefinedVal>();
                        });

                        evalCallWithFnVal(args[0]->properties["handler"], { req, res }, env);
                    }
                );

                return std::make_shared<UndefinedVal>();
            })
        },
        {
            "get",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) throw ThrowException(ArgumentError("Usage: http.get(\"http://example.com\", { headers: {} })"));

                return sendReq("GET", std::static_pointer_cast<StringVal>(args[0])->string, std::static_pointer_cast<ObjectVal>(args[1]), env);
            })
        },
        {
            "post",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) throw ThrowException(ArgumentError("Usage: http.post(\"http://example.com\", { body: \"body\", headers: {} })"));

                return sendReq("POST", std::static_pointer_cast<StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == ValueType::Object) ? std::static_pointer_cast<ObjectVal>(args[1]) : std::make_shared<ObjectVal>()), env);
            })
        },
        {
            "delete",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) throw ThrowException(ArgumentError("Usage: http.delete(\"http://example.com\", { body: \"body\", headers: {} })"));

                return sendReq("DELETE", std::static_pointer_cast<StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == ValueType::Object) ? std::static_pointer_cast<ObjectVal>(args[1]) : std::make_shared<ObjectVal>()), env);
            })
        },
        {
            "put",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) throw ThrowException(ArgumentError("Usage: http.put(\"http://example.com\", { body: \"body\", headers: {} })"));

                return sendReq("PUT", std::static_pointer_cast<StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == ValueType::Object) ? std::static_pointer_cast<ObjectVal>(args[1]) : std::make_shared<ObjectVal>()), env);
            })
        },
        {
            "patch",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) throw ThrowException(ArgumentError("Usage: http.patch(\"http://example.com\", { body: \"body\", headers: {} })"));

                return sendReq("PATCH", std::static_pointer_cast<StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == ValueType::Object) ? std::static_pointer_cast<ObjectVal>(args[1]) : std::make_shared<ObjectVal>()), env);
            })
        },
        {
            "options",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) throw ThrowException(ArgumentError("Usage: http.options(\"http://example.com\", { headers: {} })"));

                return sendReq("OPTIONS", std::static_pointer_cast<StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == ValueType::Object) ? std::static_pointer_cast<ObjectVal>(args[1]) : std::make_shared<ObjectVal>()), env);
            })
        },
        {
            "head",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::Object) throw ThrowException(ArgumentError("Usage: http.head(\"http://example.com\", { headers: {}})"));

                return sendReq("HEAD", std::static_pointer_cast<StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == ValueType::Object) ? std::static_pointer_cast<ObjectVal>(args[1]) : std::make_shared<ObjectVal>()), env);
            })
        },
    }));
};

TypePtr getTypeHttpModule()
{
    return std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
        {
            "Serve",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "context", new IdentifierType("map")) })))
        },
        {
            "get",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        },
        {
            "post",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        },
        {
            "put",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        },
        {
            "patch",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        },
        {
            "delete",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        },
        {
            "patch",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        },
        {
            "options",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        },
        {
            "head",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
        }
    })));
};