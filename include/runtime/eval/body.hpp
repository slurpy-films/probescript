#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

Val evalBody(std::vector<Stmt*> body, Env* env, bool isLoop = false) {
    Val last = std::make_shared<UndefinedVal>();
    for (Stmt* stmt : body) {
        last = eval(stmt, env);
        if (last->type == ValueType::ReturnSignal) break;
        else if (last->type == ValueType::BreakSignal && isLoop) break;
        else if (last->type == ValueType::ContinueSignal && isLoop) break;
    }
    return last;
}