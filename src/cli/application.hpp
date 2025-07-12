#pragma once
#include <vector>
#include <filesystem>
#include <algorithm>

// Probescript core includes
#include "core/frontend/ast.hpp"
#include "core/runtime/values.hpp"
#include "core/frontend/parser.hpp"
#include "core/typechecker.hpp"
#include "core/runtime/interpreter.hpp"
#include "core/modules.hpp"
#include "core/context.hpp"

#include "repl.hpp"
#include "prbtest.hpp"
#include "compiler/compiler.hpp"

extern char __PROBESCRIPTVERSION__[];

void showHelp(char* argv[]);

class Application
{
public:
    Application(int argc, char* argv[]);
    void run();

private:
    std::string m_command;
    std::vector<std::string> m_args;
    std::vector<std::string> m_flags;

    std::string m_comptarget;
    char** m_argv;
};