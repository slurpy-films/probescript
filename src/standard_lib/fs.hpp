#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include "runtime/values.hpp"
#include "types.hpp"
#include "env.hpp"
#include "errors.hpp"

namespace fs = std::filesystem;

extern fs::path g_currentCwd;

Val getValFsModule();
TypePtr getTypeFsModule();