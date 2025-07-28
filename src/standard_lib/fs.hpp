#pragma once

#include <unordered_map>
#include <string>
#include <fstream>

#include "core/vm/values.hpp"
#include "core/types.hpp"
#include "core/env.hpp"
#include "core/errors.hpp"

namespace fs = std::filesystem;

extern fs::path g_currentCwd;

namespace Probescript::Stdlib::Fs
{

VM::ValuePtr getValFsModule();
Typechecker::TypePtr getTypeFsModule();

} // namespace Probescript::Stdlib