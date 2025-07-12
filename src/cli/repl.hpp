#pragma once
#include <string>
#include <vector>
#include <iostream>

#include "core/runtime/interpreter.hpp"
#include "core/frontend/parser.hpp"
#include "core/context.hpp"
#include "core/typechecker.hpp"

class REPL
{
public:
    void start();
};