#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "runtime/interpreter.hpp"
#include "frontend/parser.hpp"
#include "context.hpp"
#include "typechecker.hpp"

class REPL
{
public:
    void start();
};
