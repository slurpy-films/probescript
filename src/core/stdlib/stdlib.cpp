#include "stdlib/stdlib.hpp"

std::unordered_map<std::string, Val> createRandomModule() {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    return {
        {
            "randInt",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2) {
                    return env->throwErr(ArgumentError("randInt expects two arguments"));
                }

                std::uniform_int_distribution<> distrib(args[0]->toNum(), args[1]->toNum());
                return std::make_shared<NumberVal>(distrib(gen));
            })
        },
        {
            "rand",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                std::uniform_real_distribution<double> distrib(0.0, 1.0);
                return std::make_shared<NumberVal>(distrib(gen));
            })
        }
    };
};

std::unordered_map<std::string, Val> getDateModule() {
    return {
        {"stamp", std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto duration = now.time_since_epoch();

        std::string unit = "sec";
        if (!args.empty() && args[0]->type == ValueType::String) {
            unit = std::static_pointer_cast<StringVal>(args[0])->string;
            std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
        }

        long long result;

        if (unit == "milli") {
            result = duration_cast<milliseconds>(duration).count();
        } else if (unit == "sec") {
            result = duration_cast<seconds>(duration).count();
        } else if (unit == "min") {
            result = duration_cast<minutes>(duration).count();
        } else if (unit == "hour") {
            result = duration_cast<hours>(duration).count();
        } else {
            return std::make_shared<StringVal>("Invalid time unit: " + unit);
        }

        return std::make_shared<NumberVal>(std::to_string(result));
    })}};
};


std::unordered_map<std::string, Val> getFilesystemModule() {
    std::unordered_map<std::string, Val> mod = {
        {"readFile", std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
            if (args.size() != 1 || args[0]->type != ValueType::String) {
                return env->throwErr(ArgumentError("readFile: Expected one string argument (file path)"));
            }

            std::string filePath = std::static_pointer_cast<StringVal>(args[0])->string;

            if (!fs::exists(filePath)) {
                return env->throwErr(ArgumentError("File does not exist: " + filePath));
            }

            if (!fs::is_regular_file(filePath)) {
                return env->throwErr(ArgumentError("Provided path is not a file: " + filePath));
            }

            std::ifstream file(filePath);
            if (!file.is_open()) {
                return env->throwErr(ArgumentError("Failed to open file: " + filePath));
            }

            std::string fileContent, line;
            while (std::getline(file, line)) {
                fileContent += line + "\n";
            }

            file.close();
            return std::make_shared<StringVal>(fileContent);
        })},

        {"writeFile", std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
            if (args.size() != 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::String) {
                return env->throwErr(ArgumentError("writeFile: Expected two string arguments (path, content)"));
            }

            std::string filePath = std::static_pointer_cast<StringVal>(args[0])->string;
            std::string content = std::static_pointer_cast<StringVal>(args[1])->string;

            std::ofstream file(filePath);
            if (!file.is_open()) {
                return env->throwErr(ArgumentError("Failed to open file for writing: " + filePath));
            }

            file << content;
            file.close();

            return std::make_shared<UndefinedVal>();
        })},

        {"exists", std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
            if (args.size() != 1 || args[0]->type != ValueType::String) {
                return env->throwErr(ArgumentError("exists: Expected one string argument (path)"));
            }

            std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
            return std::make_shared<BooleanVal>(fs::exists(path));
        })},

        {"isDirectory", std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
            if (args.size() != 1 || args[0]->type != ValueType::String) {
                return env->throwErr(ArgumentError("isDirectory: Expected one string argument (path)"));
            }

            std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
            return std::make_shared<BooleanVal>(fs::is_directory(path));
        })},

        {"listDir", std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
            if (args.size() != 1 || args[0]->type != ValueType::String) {
                return env->throwErr(ArgumentError("listDir: Expected one string argument (path)"));
            }

            std::string path = std::static_pointer_cast<StringVal>(args[0])->string;

            if (!fs::is_directory(path)) {
                return env->throwErr(ArgumentError("Provided path is not a directory: " + path));
            }

            auto array = std::make_shared<ArrayVal>();
            for (const auto& entry : fs::directory_iterator(path)) {
                array->items.push_back(std::make_shared<StringVal>(entry.path().string()));
            }

            return array;
        })},
    };

    return mod;
};

std::unordered_map<std::string, std::shared_ptr<ObjectVal>> getStdlib() {
    return {
        {"fs", std::make_shared<ObjectVal>(getFilesystemModule()) },
        {"date", std::make_shared<ObjectVal>(getDateModule()) },
        {"random", std::make_shared<ObjectVal>(createRandomModule()) },
        {"http", std::make_shared<ObjectVal>(getHttpModule()) },
        {"json", std::make_shared<ObjectVal>(getJsonModule())}
    };
};

std::unordered_map<std::string, TypePtr> getTypedStdlib()
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
};