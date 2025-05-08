#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

std::unordered_map<std::string, Val> getFilesystemModule() {
    std::unordered_map<std::string, Val> mod = {
    {"readFile", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
        if (args.size() != 1) {
            std::cerr << "readFile: Expected one argument, file path." << std::endl;
            exit(1);
        }

        std::string filePath = std::static_pointer_cast<StringVal>(args[0])->string;

        if (!fs::exists(filePath)) {
            std::cerr << "File does not exist: " << filePath << std::endl;
            exit(1);
        }

        if (!fs::is_regular_file(filePath)) {
            std::cerr << "Provided path is not a file: " << filePath << std::endl;
            exit(1);
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            exit(1);
        }

        std::string fileContent;
        std::string line;
        while (std::getline(file, line)) {
            fileContent += line + "\n";
        }

        file.close();

        return std::make_shared<StringVal>(fileContent);
    })},
    {"writeFile", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
        if (args.size() != 2 || args[0]->type != ValueType::String || args[1]->type != ValueType::String) {
            std::cerr << "writeFile: Expected two string arguments (path, content)." << std::endl;
            exit(1);
        }
    
        std::string filePath = std::static_pointer_cast<StringVal>(args[0])->string;
        std::string content = std::static_pointer_cast<StringVal>(args[1])->string;
    
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            exit(1);
        }
    
        file << content;
        file.close();
    
        return std::make_shared<UndefinedVal>();
    })},
    {"exists", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
        if (args.size() != 1 || args[0]->type != ValueType::String) {
            std::cerr << "exists: Expected one string argument (path)." << std::endl;
            exit(1);
        }
    
        std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
        return std::make_shared<BooleanVal>(fs::exists(path));
    })},
    {"isDirectory", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
        if (args.size() != 1 || args[0]->type != ValueType::String) {
            std::cerr << "isDirectory: Expected one string argument (path)." << std::endl;
            exit(1);
        }
    
        std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
        return std::make_shared<BooleanVal>(fs::is_directory(path));
    })},
    {"listDir", std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
        if (args.size() != 1 || args[0]->type != ValueType::String) {
            std::cerr << "listDir: Expected one string argument (path)." << std::endl;
            exit(1);
        }
    
        std::string path = std::static_pointer_cast<StringVal>(args[0])->string;
    
        if (!fs::is_directory(path)) {
            std::cerr << "Provided path is not a directory: " << path << std::endl;
            exit(1);
        }
    
        std::shared_ptr<ArrayVal> array = std::make_shared<ArrayVal>();
    
        for (const auto& entry : fs::directory_iterator(path)) {
            array->items.push_back(std::make_shared<StringVal>(entry.path().string()));
        }
    
        return array;
    })},            
    };

    return mod;
};
