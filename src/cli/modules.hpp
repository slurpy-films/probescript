#pragma once

#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <fstream>

#include "json.hpp"

#include "core/runtime/values.hpp"

namespace fs = std::filesystem;
namespace Probescript::ModuleIndexer
{
    
std::pair<std::unordered_map<std::string, fs::path>, Values::Val> indexModules(fs::path fileName);

} // namespace Probescript::ModuleIndexer