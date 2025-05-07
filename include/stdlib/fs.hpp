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
    })}};

    return mod;
};
