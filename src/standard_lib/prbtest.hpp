#pragma once
#include <string>
#include <vector>
#include <functional>

#include "core/runtime/values.hpp"
#include "core/runtime/interpreter.hpp"
#include "core/env.hpp"

struct TestCase
{
    std::string name;
    std::function<void()> fn;

    TestCase(std::string name, std::function<void()> fn)
        : name(name), fn(fn) {};
};

extern std::vector<TestCase> g_tests;

Val getValTestLib();
TypePtr getTypeTestLib();

void runTests(std::string file);