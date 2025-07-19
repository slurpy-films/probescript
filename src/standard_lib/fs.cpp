#include "fs.hpp"

using namespace Probescript;
using namespace Probescript::Stdlib;

Values::Val Fs::getValFsModule()
{
    return std::make_shared<Values::ObjectVal>(std::unordered_map<std::string, Values::Val>(
    {
        {
            "read_file",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.size() != 1 || args[0]->type != Values::ValueType::String)
                {
                    throw ThrowException(ArgumentError("readFile: Expected one string argument (file path)"));
                }

                fs::path filePath = g_currentCwd / fs::path(std::static_pointer_cast<Values::StringVal>(args[0])->string);

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
                return std::make_shared<Values::StringVal>(fileContent);
        })
        },
        {
            "write_file",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.size() != 2 || args[0]->type != Values::ValueType::String || args[1]->type != Values::ValueType::String)
                {
                    throw ThrowException(ArgumentError("writeFile: Expected two string arguments (path, content)"));
                }

                fs::path filePath = g_currentCwd / fs::path(std::static_pointer_cast<Values::StringVal>(args[0])->string);
                std::string content = std::static_pointer_cast<Values::StringVal>(args[1])->string;

                std::ofstream file(filePath);
                if (!file.is_open())
                {
                    throw ThrowException(ArgumentError("Failed to open file for writing: " + filePath.string()));
                }

                file << content;
                file.close();

                return std::make_shared<Values::UndefinedVal>();
            }
        )
        },
        {
            "exists",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.size() != 1 || args[0]->type != Values::ValueType::String)
                {
                    throw ThrowException(ArgumentError("exists: Expected one string argument (path)"));
                }

                std::string path = std::static_pointer_cast<Values::StringVal>(args[0])->string;
                return std::make_shared<Values::BooleanVal>(fs::exists(path));
            })
        },
        {
            "is_directory",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.size() != 1 || args[0]->type != Values::ValueType::String)
                {
                    throw ThrowException(ArgumentError("isDirectory: Expected one string argument (path)"));
                }

                std::string path = std::static_pointer_cast<Values::StringVal>(args[0])->string;
                return std::make_shared<Values::BooleanVal>(fs::is_directory(path));
            })
        },
        {
            "list_dir",
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.size() != 1 || args[0]->type != Values::ValueType::String)
                {
                    throw ThrowException(ArgumentError("listDir: Expected one string argument (path)"));
                }

                std::string path = std::static_pointer_cast<Values::StringVal>(args[0])->string;

                if (!fs::is_directory(path))
                {
                    throw ThrowException(ArgumentError("Provided path is not a directory: " + path));
                }

                auto array = std::make_shared<Values::ArrayVal>();
                for (const auto& entry : fs::directory_iterator(path))
                {
                    array->items.push_back(std::make_shared<Values::StringVal>(entry.path().string()));
                }

                return array;
            })
        }
    }));
}

Typechecker::TypePtr Fs::getTypeFsModule()
{
    return std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Module, "native module", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>({
        {
            "read_file",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("path", Typechecker::g_strty, false) })))
        },
        {
            "write_file",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("path", Typechecker::g_strty, false), std::make_shared<Typechecker::Parameter>("content", Typechecker::g_strty, false) })))
        },
        {
            "exists",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("path", Typechecker::g_strty, false) })))
        },
        {
            "is_directory",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("path", Typechecker::g_strty, false) })))
        },
        {
            "list_dir",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("path", Typechecker::g_strty, false) })))
        }
    })));
}