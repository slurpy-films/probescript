#pragma once
#include <vector>

#include "frontend/ast.hpp"
#include "runtime/values.hpp"
#include "frontend/parser.hpp"
#include "typechecker.hpp"
#include "runtime/interpreter.hpp"
#include "repl.hpp"
#include "modules.hpp"
#include "context.hpp"

extern char __PROBESCRIPTVERSION__[];

void showHelp(char* argv[]);

class Application
{
public:
    Application(int argc, char* argv[]);
    void run();

private:
    int argc;
    char** argv;
};