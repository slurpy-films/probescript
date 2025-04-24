#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "runprobe.hpp"

RuntimeVal* evalProgram(ProgramType* program, Env* env, string probeName) {
    for (Stmt* stmt : program->body) {
        eval(stmt, env);
    }

    RuntimeVal* lastEval = evalProbeCall(probeName, env);

    return lastEval;
}