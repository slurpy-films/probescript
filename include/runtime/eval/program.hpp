#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalProgram(ProgramType* program, Env* env) {
    RuntimeVal* lastEval = new NullVal();

    for (Stmt* stmt : program->body) {
        lastEval = eval(stmt, env);
    }

    return lastEval;
}