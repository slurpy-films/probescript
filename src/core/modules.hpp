#pragma once
#include "runtime/values.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <fstream>
#include "json.hpp"
namespace fs = std::filesystem;

std::pair<std::unordered_map<std::string, fs::path>, Val> indexModules(fs::path fileName);