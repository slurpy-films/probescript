#pragma once

#include <string>
#include <vector>
#include <functional>

#include "core/vm/values.hpp"
#include "core/vm/vm.hpp"

#include "core/types.hpp"

namespace Probescript::Stdlib::Prbtest
{

struct TestCase
{
    std::string name;
    std::function<void()> fn;

    TestCase(std::string name, std::function<void()> fn)
        : name(name), fn(fn) {};
};


VM::ValuePtr getValTestLib();
Typechecker::TypePtr getTypeTestLib();

void runTests(std::string file);

} // namespace Probescript::Stdlib::Prbtest

extern std::vector<Probescript::Stdlib::Prbtest::TestCase> g_tests;