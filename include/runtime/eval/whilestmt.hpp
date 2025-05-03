#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "body.hpp"

RuntimeVal* evalWhileStmt(WhileStmtType* stmt, Env* env) {
    RuntimeVal* res = new UndefinedVal();
    while (true) {
        RuntimeVal* result = eval(stmt->condition, env);
        if (result->type != ValueType::Boolean) {
            std::cerr << "While condition must evaluate to a boolean";
            exit(1);
        }

        if (static_cast<BooleanVal*>(result)->getValue()) {
            Env* scope = new Env(env);
            res = evalBody(stmt->body, scope);
            if (res->type == ValueType::ReturnSignal) break;

        } else break;
    }

    return res;
}