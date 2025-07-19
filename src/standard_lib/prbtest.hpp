#pragma once

#include <string>
#include <vector>
#include <functional>

#include "core/runtime/values.hpp"
#include "core/runtime/interpreter.hpp"
#include "core/env.hpp"

namespace Probescript::Stdlib::Prbtest
{

struct TestCase
{
    std::string name;
    std::function<void()> fn;

    TestCase(std::string name, std::function<void()> fn)
        : name(name), fn(fn) {};
};


Values::Val getValTestLib();
Typechecker::TypePtr getTypeTestLib();

void runTests(std::string file);

} // namespace Probescript::Stdlib::Prbtest

extern std::vector<Probescript::Stdlib::Prbtest::TestCase> g_tests;