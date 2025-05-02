#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <fstream>

namespace fs = std::filesystem;

std::unordered_map<std::string, fs::path> indexModules(fs::path fileName) {
    fs::path current = fs::current_path() / fileName.parent_path();
    fs::path projectFile;

    for (size_t i = 0; i < 10; ++i) {
        fs::path candidate = current / "project.probe";

        if (fs::exists(candidate)) {
            projectFile = candidate;
            break;
        }

        if (current.has_parent_path()) {
            current = current.parent_path();
        } else {
            std::cerr << "Could not find project file";
            exit(1);
        }
    }

    std::unordered_map<std::string, fs::path> modules;

    for (const auto& entry : fs::recursive_directory_iterator(projectFile.parent_path())) {
        if (entry.is_regular_file()) {
            fs::path path = entry.path();

            if (path.extension() == ".probe") {
                std::ifstream file(path);

                if (file.is_open()) {
                    std::string firstLine;
                    std::getline(file, firstLine);

                    if (firstLine.find("module") == 0) {
                        std::string modulename;
                        if (firstLine.find(";") != std::string::npos) {
                            modulename = firstLine.substr(7, firstLine.find(";") - 7);
                        } else {
                            modulename = firstLine.substr(7);
                        }

                        modules[modulename] = path;
                    }
                }
            }
        }
    }

    return modules;
}