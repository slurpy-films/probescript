#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "core/runtime/interpreter.hpp"
#include "core/frontend/parser.hpp"
#include "core/typechecker.hpp"
#include "context.hpp"

namespace Probescript
{

class REPL
{
public:
    void start();
};

} // namespace Probescript