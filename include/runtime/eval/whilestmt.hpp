#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

RuntimeVal* evalWhileStmt(WhileStmtType* stmt, Env* env) {
    while (true) {
        RuntimeVal* result = eval(stmt->condition, env);
        if (result->type != ValueType::Boolean) {
            std::cerr << "While condition must evaluate to a boolean";
            exit(1);
        }

        if (static_cast<BooleanVal*>(result)->getValue()) {
            Env* scope = new Env(env);
            for (Stmt* stmt : stmt->body) {
                eval(stmt, scope);
            }
        } else break;
    }

    return new UndefinedVal();
}