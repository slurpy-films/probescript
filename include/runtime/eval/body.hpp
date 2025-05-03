#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

RuntimeVal* evalBody(std::vector<Stmt*> body, Env* env) {
    RuntimeVal* last = new UndefinedVal();
    for (Stmt* stmt : body) {
        last = eval(stmt, env);
        if (last->type == ValueType::ReturnSignal) break;
    }
    return last;
}