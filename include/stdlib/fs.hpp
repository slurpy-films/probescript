#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

unordered_map<string, RuntimeVal*> filesystemModule = {
    { "readFile", new NativeFnValue([](vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
        if (args.size() != 1) {
            cerr << "readFile: Expected one argument, file path." << endl;
            exit(1);
        }

        std::string filePath = static_cast<StringVal*>(args[0])->string;

        if (!fs::exists(filePath)) {
            cerr << "File does not exist: " << filePath << endl;
            exit(1);
        }

        if (!fs::is_regular_file(filePath)) {
            cerr << "Provided path is not a file: " << filePath << endl;
            exit(1);
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filePath << endl;
            exit(1);
        }

        std::string fileContent;
        std::string line;
        while (std::getline(file, line)) {
            fileContent += line + "\n";
        }

        file.close();

        return new StringVal(fileContent);
    })}
};
