#include "http.hpp"

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

using namespace Probescript;
using namespace Probescript::Stdlib;
using namespace Probescript::Stdlib::Http;

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

Values::Val sendReq(const std::string& method, std::string& url, std::shared_ptr<Values::ObjectVal> conf, EnvPtr env)
{
    std::string headers;
    if (conf->hasProperty("headers") && conf->properties["headers"]->type == Values::ValueType::Object)
    {
        for (auto& [key, val] : std::static_pointer_cast<Values::ObjectVal>(conf->properties["headers"])->properties)
        {
            if (val->type == Values::ValueType::String)
            {
                headers += key + ": " + std::static_pointer_cast<Values::StringVal>(val)->string + "\r\n";
            }
        }
    }

    std::string body;
    if (conf->hasProperty("body") && conf->properties["body"]->type == Values::ValueType::String)
    {
        body = std::static_pointer_cast<Values::StringVal>(conf->properties["body"])->string;
    }

    std::regex urlRegex(R"(^(http?://)?([^:/]+)(:(\d+))?(/.*)?$)");
    std::smatch match;
    if (!std::regex_match(url, match, urlRegex))
    {
        throw ThrowException("[HttpError]: Invalid URL format: " + url);
    }

    int port = 80;
    if (match[4].matched)
    {
        try
        {
            port = std::stoi(match[4]);
        }
        catch (...)
        {
            throw ThrowException("[HttpError]: Invalid port number in URL: " + match[4].str());
        }
    }

    std::string host = match[2];
    std::string path = match[5].matched ? match[5].str() : "/";
    std::ostringstream reqStream;
    reqStream << method << " " << path << " HTTP/1.1\r\n"
              << "Host: " << host << "\r\n"
              << "Connection: close\r\n"
              << "Content-length: " << body.length() << "\r\n"
              << headers << "\r\n"
              << body;

    std::string req = reqStream.str();

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    struct hostent *server = gethostbyname(host.c_str());
    if (!server)
    {
        throw ThrowException("[HttpError]: Failed to resolve host: " + host);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        throw ThrowException("[HttpError]: Socket creation failed");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = *(unsigned long*)server->h_addr;

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        throw ThrowException("[HttpError]: Connection to " + host + ":" + std::to_string(port) + " failed");
    }

    send(sock, req.c_str(), req.size(), 0);

    char buffer[4096];
    std::string res;
    int bytes;
    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
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
    if (headerEnd == std::string::npos)
    {
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
    try
    {
        statusCode = std::stoi(statusCodeStr);
    }
    catch (...)
    {
        throw ThrowException("[HttpError]: Invalid status code: " + statusCodeStr);
    }

    // Parse headers into ObjectVal
    auto headerMap = std::make_shared<Values::ObjectVal>();
    std::string line;
    while (std::getline(stream, line))
    {
        if (line.empty() || line == "\r") continue;
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string val = line.substr(colonPos + 1);
            val = std::regex_replace(val, std::regex("^ +"), "");
            val = std::regex_replace(val, std::regex("\r$"), "");
            headerMap->properties[key] = std::make_shared<Values::StringVal>(val);
        }
    }

    std::unordered_map<std::string, Values::Val> props = {
        { "status", std::make_shared<Values::NumberVal>(statusCode) },
        { "headers", headerMap },
        { "body", std::make_shared<Values::NativeFnValue>([bodyPart](std::vector<Values::Val>, EnvPtr) -> Values::Val {
            return std::make_shared<Values::StringVal>(bodyPart);
        }) }
    };

    return std::make_shared<Values::ObjectVal>(props);
}

Values::Val Http::getValHttpModule()
{
    return std::make_shared<Values::ObjectVal>(std::unordered_map<std::string, Values::Val>({
        {
            "Serve",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (
                    args.size() < 2
		    || args[0]->type != Values::ValueType::Number
		    || args[1]->type != Values::ValueType::Function
                ) throw ThrowException(ArgumentError("Usage: http.Serve(port: number, handler: function)"));

                startServer(
                    std::static_pointer_cast<Values::NumberVal>(args[0])->number,
                    [args, env](std::shared_ptr<Request> request, std::shared_ptr<Response> response) -> void
                    {
                        std::shared_ptr<Values::ObjectVal> req = std::make_shared<Values::ObjectVal>();
                        std::shared_ptr<Values::ObjectVal> res = std::make_shared<Values::ObjectVal>();

                        req->properties["path"] = std::make_shared<Values::StringVal>(request->path);
                        req->properties["method"] = std::make_shared<Values::StringVal>(request->method);
                        req->properties["headers"] = std::make_shared<Values::ObjectVal>();
                        req->properties["cookies"] = std::make_shared<Values::ObjectVal>();

                        req->properties["ondata"] = std::make_shared<Values::NativeFnValue>([request](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
                        {
                            if (args.empty() || args[0]->type != Values::ValueType::Function) 
                                throw ThrowException(ArgumentError("Usage: req.ondata(callback: function)"));
                            
                            request->ondata = std::function<void(std::string)>([args, env](std::string data)
                            {
                                Interpreter::evalCallWithFnVal(args[0], { std::make_shared<Values::StringVal>(data) }, env);
                            });

                            return std::make_shared<Values::UndefinedVal>();
                        });

                        req->properties["end"] = std::make_shared<Values::NativeFnValue>([request](std::vector<Values::Val> args, EnvPtr _env) -> Values::Val
                        {
                            if (args.empty() || args[0]->type != Values::ValueType::Function) 
                                throw ThrowException(ArgumentError("Usage: req.end(callback: function)"));
                            
                            request->end = std::function<void()>([args, _env]()
                            {
                                Interpreter::evalCallWithFnVal(args[0], {}, _env);
                            });

                            return std::make_shared<Values::UndefinedVal>();
                        });

                        for (const auto& [key, val] : request->headers)
                            req->properties["headers"]->properties[key] = std::make_shared<Values::StringVal>(val);

                        for (const auto& [key, val] : request->cookies)
                            req->properties["cookies"]->properties[key] = std::make_shared<Values::StringVal>(val);

                        req->properties["raw"] =
                        std::make_shared<Values::NativeFnValue>([request](std::vector<Values::Val> _args, EnvPtr _env) -> Values::Val
                        {
                            return std::make_shared<Values::StringVal>(request->raw);
                        });

                        auto resheaders = std::make_shared<std::unordered_map<std::string, std::string>>();
                        (*resheaders)["Content-Type"] = "text/plain";

                        res->properties["content_type"] = std::make_shared<Values::NativeFnValue>([resheaders](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
                        {
                            if (args.empty()) throw ThrowException(ArgumentError("Usage: res.content_type(type: str)"));

                            (*resheaders)["Content-Type"] = args[0]->toString();

                            return std::make_shared<Values::UndefinedVal>();
                        });

                        res->properties["header"] = std::make_shared<Values::NativeFnValue>([resheaders](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
                        {
                            if (args.empty()) throw ThrowException(ArgumentError("Usage: res.header(key: str, value: str)"));

                            (*resheaders)[args[0]->toString()] = args[1]->toString();

                            return std::make_shared<Values::UndefinedVal>();
                        });

                        res->properties["send"] = std::make_shared<Values::NativeFnValue>([resheaders, response](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
                        {
                            if (args.empty()) throw ThrowException(ArgumentError("Usage: res.send(body: str)"));

                            response->send(args[0]->toString(), (*resheaders));

                            return std::make_shared<Values::UndefinedVal>();
                        });

                        res->properties["html"] = std::make_shared<Values::NativeFnValue>([resheaders, response](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
                        {
                            if (args.empty()) throw ThrowException(ArgumentError("Usage: res.html(html: str)"));

                            (*resheaders)["Content-Type"] = "text/html";
                            response->send(args[0]->toString(), (*resheaders));

                            return std::make_shared<Values::UndefinedVal>();
                        });

                        res->properties["json"] = std::make_shared<Values::NativeFnValue>([resheaders, response](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
                        {
                            if (args.empty() || args[0]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: res.html(object: object)"));

                            (*resheaders)["Content-Type"] = "application/json";
                            response->send(args[0]->toString(), (*resheaders));

                            return std::make_shared<Values::UndefinedVal>();
                        });

                        Interpreter::evalCallWithFnVal(args[1], { req, res }, env);
                    }
                );

                return std::make_shared<Values::UndefinedVal>();
            })
        },
        {
            "get",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (args.size() < 2 || args[0]->type != Values::ValueType::String || args[1]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: http.get(\"http://example.com\", { headers: {} })"));
                
                return std::make_shared<Values::FutureVal>(std::async(std::launch::async, [args, env]() -> Values::Val
                {
                    return sendReq("GET", std::static_pointer_cast<Values::StringVal>(args[0])->string, std::static_pointer_cast<Values::ObjectVal>(args[1]), env);
                }));
            })
        },
        {
            "post",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (args.size() < 2 || args[0]->type != Values::ValueType::String || args[1]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: http.post(\"http://example.com\", { body: \"body\", headers: {} })"));
                
                return std::make_shared<Values::FutureVal>(std::async(std::launch::async, [args, env]() -> Values::Val
                {
                    return sendReq("POST", std::static_pointer_cast<Values::StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == Values::ValueType::Object) ? std::static_pointer_cast<Values::ObjectVal>(args[1]) : std::make_shared<Values::ObjectVal>()), env);
                }));
            })
        },
        {
            "delete",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (args.size() < 2 || args[0]->type != Values::ValueType::String || args[1]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: http.delete(\"http://example.com\", { body: \"body\", headers: {} })"));
                
                return std::make_shared<Values::FutureVal>(std::async(std::launch::async, [args, env]() -> Values::Val
                {
                    return sendReq("DELETE", std::static_pointer_cast<Values::StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == Values::ValueType::Object) ? std::static_pointer_cast<Values::ObjectVal>(args[1]) : std::make_shared<Values::ObjectVal>()), env);
                }));
            })
        },
        {
            "put",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (args.size() < 2 || args[0]->type != Values::ValueType::String || args[1]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: http.put(\"http://example.com\", { body: \"body\", headers: {} })"));
                
                return std::make_shared<Values::FutureVal>(std::async(std::launch::async, [args, env]() -> Values::Val
                {    
                    return sendReq("PUT", std::static_pointer_cast<Values::StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == Values::ValueType::Object) ? std::static_pointer_cast<Values::ObjectVal>(args[1]) : std::make_shared<Values::ObjectVal>()), env);
                }));
            })
        },
        {
            "patch",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (args.size() < 2 || args[0]->type != Values::ValueType::String || args[1]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: http.patch(\"http://example.com\", { body: \"body\", headers: {} })"));
                
                return std::make_shared<Values::FutureVal>(std::async(std::launch::async, [args, env]() -> Values::Val
                {
                    return sendReq("PATCH", std::static_pointer_cast<Values::StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == Values::ValueType::Object) ? std::static_pointer_cast<Values::ObjectVal>(args[1]) : std::make_shared<Values::ObjectVal>()), env);
                }));
            })
        },
        {
            "options",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (args.size() < 2 || args[0]->type != Values::ValueType::String || args[1]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: http.options(\"http://example.com\", { headers: {} })"));
                
                return std::make_shared<Values::FutureVal>(std::async(std::launch::async, [args, env]() -> Values::Val
                {
                    return sendReq("OPTIONS", std::static_pointer_cast<Values::StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == Values::ValueType::Object) ? std::static_pointer_cast<Values::ObjectVal>(args[1]) : std::make_shared<Values::ObjectVal>()), env);
                }));
            })
        },
        {
            "head",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (args.size() < 2 || args[0]->type != Values::ValueType::String || args[1]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: http.head(\"http://example.com\", { headers: {} })"));
                
                return std::make_shared<Values::FutureVal>(std::async(std::launch::async, [args, env]() -> Values::Val
                {
                    return sendReq("HEAD", std::static_pointer_cast<Values::StringVal>(args[0])->string, ((args.size() > 1 && args[1]->type == Values::ValueType::Object) ? std::static_pointer_cast<Values::ObjectVal>(args[1]) : std::make_shared<Values::ObjectVal>()), env);
                }));
            })
        },
        {
            "Request",
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                return std::make_shared<Values::UndefinedVal>();
            })
        },
        {
            "Response",
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                return std::make_shared<Values::UndefinedVal>();
            })
        }
    }));
};

Typechecker::TypePtr Http::getTypeHttpModule()
{
    return std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Module, "native module", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>({
        {
            "Serve",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("port", Typechecker::g_numty, false),
                        std::make_shared<Typechecker::Parameter>("handler", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function"),
                        false)
                    })
                )
            )
        },
        {
            "get",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("url", Typechecker::g_strty, false),
                        std::make_shared<Typechecker::Parameter>("req", Typechecker::g_mapty, false) }
                    ),
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")
                )
            )
        },
        {
            "post",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("url", Typechecker::g_strty, false),
                        std::make_shared<Typechecker::Parameter>("req", Typechecker::g_mapty, false)
                    }),
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")
                )
            )
        },
        {
            "put",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("url", Typechecker::g_strty, false),
                        std::make_shared<Typechecker::Parameter>("req", Typechecker::g_mapty, false) 
                    }),
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")
                )
            )
        },
        {
            "patch",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("url", Typechecker::g_strty, false),
                        std::make_shared<Typechecker::Parameter>("req", Typechecker::g_mapty, false)
                    }),
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")
                )
            )
        },
        {
            "delete",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("url", Typechecker::g_strty, false),
                        std::make_shared<Typechecker::Parameter>("req", Typechecker::g_mapty, false)
                    }),
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")
                )
            )
        },
        {
            "patch",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("url", Typechecker::g_strty, false),
                        std::make_shared<Typechecker::Parameter>("req", Typechecker::g_mapty, false)
                    }),
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")
                )
            )
        },
        {
            "options",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("url", Typechecker::g_strty, false),
                        std::make_shared<Typechecker::Parameter>("req", Typechecker::g_mapty, false)
                    }),
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")
                )
            )
        },
        {
            "head",
            std::make_shared<Typechecker::Type>(
                Typechecker::TypeKind::Function,
                "native function",
                std::make_shared<Typechecker::TypeVal>(
                    std::vector({
                        std::make_shared<Typechecker::Parameter>("url", Typechecker::g_strty, false),
                        std::make_shared<Typechecker::Parameter>("req", Typechecker::g_mapty, false)
                    }),
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")
                )
            )
        },
        {
            "Request",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>(
            {
                {
                    "method",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::String, "string")
                },
                {
                    "path",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::String, "string")
                },
                {
                    "ondata",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("function", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function"), false) })))
                },
                {
                    "end",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("function", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function"), false) })))
                },
                {
                    "cookies",
                    Typechecker::g_anyty
                },
                {
                    "headers",
                    Typechecker::g_anyty
                },
                {
                    "raw",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(Typechecker::g_strty))
                },
                {
                    "content_type",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("type", Typechecker::g_strty, false) })))
                }
            })), "Request")
        },
        {
            "Response",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>(
            {
                {
                    "json",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("object", Typechecker::g_anyty, false) })))
                },
                {
                    "send",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("data", Typechecker::g_strty, false) })))
                },
                {
                    "html",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("html", Typechecker::g_strty, false) })))
                },
                {
                    "content_type",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("type", Typechecker::g_strty, false) })))
                },
                {
                    "header",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("key", Typechecker::g_strty, false), std::make_shared<Typechecker::Parameter>("val", Typechecker::g_strty, false) })))
                }
            })), "Response")
        }
    })));
};