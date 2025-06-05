#pragma once
#include <filesystem>
#include <fstream>
#include "runtime/values.hpp"
#include "env.hpp"
#include "runtime/interpreter.hpp"
#include "frontend/ast.hpp"
#include "frontend/parser.hpp"
#include "repl.hpp"
#include "context.hpp"
#include "modules.hpp"
#include "init.hpp"
#include "typechecker.hpp"

void showHelp(char* argv[]);
int main(int argc, char* argv[]);