#pragma once

#include <vector>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <unordered_set>

#include "core/frontend/ast.hpp"
#include "core/runtime/values.hpp"
#include "core/frontend/parser.hpp"
#include "core/typechecker.hpp"
#include "core/runtime/interpreter.hpp"

#include "core/vm/vm.hpp"
#include "core/vm/values.hpp"

#include "core/compiler/compiler.hpp"

#include "repl.hpp"
#include "modules.hpp"
#include "context.hpp"
#include "prbtest.hpp"

#include "VERSION.h"

void showHelp(char* argv[]);

class Application
{
public:
    Application(int argc, char* argv[]);
    void run();

private:
    std::string m_command;
    std::vector<std::string> m_args;
    std::unordered_set<std::string> m_flags;
    char** m_argv;
};