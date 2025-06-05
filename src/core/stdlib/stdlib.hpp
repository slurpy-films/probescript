#pragma once
#include <string>
#include <unordered_map>
#include <random>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <string>
#include <algorithm>

#include "runtime/values.hpp"
#include "stdlib/http.hpp"
#include "stdlib/json.hpp"
#include "typechecker.hpp"
#include "runtime/values.hpp"
#include "env.hpp"

std::unordered_map<std::string, Val> createRandomModule();
std::unordered_map<std::string, Val> getFilesystemModule();
std::unordered_map<std::string, Val> getDateModule();

std::unordered_map<std::string, std::shared_ptr<ObjectVal>> getStdlib();
std::unordered_map<std::string, TypePtr> getTypedStdlib();