#pragma once
#include <vector>
#include <filesystem>
#include <algorithm>

#include "frontend/ast.hpp"
#include "runtime/values.hpp"
#include "frontend/parser.hpp"
#include "typechecker.hpp"
#include "runtime/interpreter.hpp"
#include "repl.hpp"
#include "modules.hpp"
#include "context.hpp"
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