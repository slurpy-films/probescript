#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "errors.hpp"
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

inline std::unordered_map<std::string, Val> getFilesystemModule() {
    std::unordered_map<std::string, Val> mod = {
        {"readFile", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
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

        {"writeFile", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
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

        {"exists", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
            if (args.size() != 1 || args[0]->type != ValueType::String) {
                return env->throwErr(ArgumentError("exists: Expected one string argument (path)"));
            }

            std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
            return std::make_shared<BooleanVal>(fs::exists(path));
        })},

        {"isDirectory", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
            if (args.size() != 1 || args[0]->type != ValueType::String) {
                return env->throwErr(ArgumentError("isDirectory: Expected one string argument (path)"));
            }

            std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
            return std::make_shared<BooleanVal>(fs::is_directory(path));
        })},

        {"listDir", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
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
}
