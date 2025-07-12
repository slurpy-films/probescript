#pragma once
#include <unordered_map>
#include <string>
#include <fstream>

#include "core/runtime/values.hpp"
#include "core/types.hpp"
#include "core/env.hpp"
#include "core/errors.hpp"

namespace fs = std::filesystem;

extern fs::path g_currentCwd;

Val getValFsModule();
TypePtr getTypeFsModule();