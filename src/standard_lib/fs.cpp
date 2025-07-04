#include "fs.hpp"

Val getValFsModule()
{
    return std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>(
    {
        {
            "read_file",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.size() != 1 || args[0]->type != ValueType::String)
                {
                    throw ThrowException(ArgumentError("readFile: Expected one string argument (file path)"));
                }

                fs::path filePath = g_currentCwd / fs::path(std::static_pointer_cast<StringVal>(args[0])->string);

                if (!fs::exists(filePath))
                {
                    throw ThrowException(ArgumentError("File does not exist: " + filePath.string()));
                }

                if (!fs::is_regular_file(filePath))
                {
                    throw ThrowException(ArgumentError("Provided path is not a file: " + filePath.string()));
                }

                std::ifstream file(filePath);
                if (!file.is_open()) {
                    throw ThrowException(ArgumentError("Failed to open file: " + filePath.string()));
                }

                std::string fileContent, line;
                while (std::getline(file, line))
                {
                    fileContent += line + "\n";
                }

                file.close();
                return std::make_shared<StringVal>(fileContent);
        })
        },
        {
            "write_file",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.size() != 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::String)
                {
                    throw ThrowException(ArgumentError("writeFile: Expected two string arguments (path, content)"));
                }

                fs::path filePath = g_currentCwd / fs::path(std::static_pointer_cast<StringVal>(args[0])->string);
                std::string content = std::static_pointer_cast<StringVal>(args[1])->string;

                std::ofstream file(filePath);
                if (!file.is_open())
                {
                    throw ThrowException(ArgumentError("Failed to open file for writing: " + filePath.string()));
                }

                file << content;
                file.close();

                return std::make_shared<UndefinedVal>();
            }
        )
        },
        {
            "exists",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.size() != 1 || args[0]->type != ValueType::String)
                {
                    throw ThrowException(ArgumentError("exists: Expected one string argument (path)"));
                }

                std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                return std::make_shared<BooleanVal>(fs::exists(path));
            })
        },
        {
            "is_directory",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.size() != 1 || args[0]->type != ValueType::String)
                {
                    throw ThrowException(ArgumentError("isDirectory: Expected one string argument (path)"));
                }

                std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
                return std::make_shared<BooleanVal>(fs::is_directory(path));
            })
        },
        {
            "list_dir",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.size() != 1 || args[0]->type != ValueType::String)
                {
                    throw ThrowException(ArgumentError("listDir: Expected one string argument (path)"));
                }

                std::string path = std::static_pointer_cast<StringVal>(args[0])->string;

                if (!fs::is_directory(path))
                {
                    throw ThrowException(ArgumentError("Provided path is not a directory: " + path));
                }

                auto array = std::make_shared<ArrayVal>();
                for (const auto& entry : fs::directory_iterator(path))
                {
                    array->items.push_back(std::make_shared<StringVal>(entry.path().string()));
                }

                return array;
            })
        }
    }));
}

TypePtr getTypeFsModule()
{
    return std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
        {
            "read_file",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ std::make_shared<VarDeclarationType>(std::make_shared<UndefinedLiteralType>(), "path", std::make_shared<IdentifierType>("str")) })))
        },
        {
            "write_file",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ std::make_shared<VarDeclarationType>(std::make_shared<UndefinedLiteralType>(), "path", std::make_shared<IdentifierType>("str")), std::make_shared<VarDeclarationType>(std::make_shared<UndefinedLiteralType>(), "path", std::make_shared<IdentifierType>("str")) })))
        },
        {
            "exists",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ std::make_shared<VarDeclarationType>(std::make_shared<UndefinedLiteralType>(), "path", std::make_shared<IdentifierType>("str")) })))
        },
        {
            "is_directory",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ std::make_shared<VarDeclarationType>(std::make_shared<UndefinedLiteralType>(), "path", std::make_shared<IdentifierType>("str")) })))
        },
        {
            "list_dir",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ std::make_shared<VarDeclarationType>(std::make_shared<UndefinedLiteralType>(), "path", std::make_shared<IdentifierType>("str")) })))
        }
    })));
}