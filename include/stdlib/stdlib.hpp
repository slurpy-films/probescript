#pragma once
#include <string>
#include <unordered_map>
#include "fs.hpp"
#include "date.hpp"
#include "runtime/values.hpp"
#include "random.hpp"
#include "http.hpp"
#include "json.hpp"
#include "typechecker.hpp"

inline std::unordered_map<std::string, std::shared_ptr<ObjectVal>> getStdlib() {
    return {
        {"fs", std::make_shared<ObjectVal>(getFilesystemModule()) },
        {"date", std::make_shared<ObjectVal>(getDateModule()) },
        {"random", std::make_shared<ObjectVal>(createRandomModule()) },
        {"http", std::make_shared<ObjectVal>(getHttpModule()) },
        {"json", std::make_shared<ObjectVal>(getJsonModule())}
    };
};

inline std::unordered_map<std::string, TypePtr> getTypedStdlib()
{
    return {
        {
            "http",
            std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
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
            })))
        },
        {
            "json",
            std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
                {
                    "parse",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "raw", new IdentifierType("str")) })))
                },
                {
                    "stringify",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "object") })))
                }
            })))
        },
        {
            "random",
            std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
                {
                    "randInt",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "x"), new VarDeclarationType(new UndefinedLiteralType(), "y") })))
                },
                {
                    "rand",
                    std::make_shared<Type>(TypeKind::Function, "native function")
                }
            })))
        },
        {
            "fs",
            std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
                {
                    "readFile",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                },
                {
                    "writeFile",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                },
                {
                    "exists",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                },
                {
                    "isDirectory",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                },
                {
                    "listDir",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                }
            })))
        },
        {
            "date",
            std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
                {
                    "stamp",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new StringLiteralType("sec"), "unit", new IdentifierType("str")) })))
                }
            })))
        }
    };
}