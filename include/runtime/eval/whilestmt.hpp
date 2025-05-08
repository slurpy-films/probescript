#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "body.hpp"

Val evalWhileStmt(WhileStmtType* stmt, Env* env) {
    Val res = std::make_shared<UndefinedVal>();
    while (true) {
        Val result = eval(stmt->condition, env);
        if (result->type != ValueType::Boolean) {
            std::cerr << "While condition must evaluate to a boolean";
            exit(1);
        }

        if (std::static_pointer_cast<BooleanVal>(result)->getValue()) {
            Env* scope = new Env(env);
            res = evalBody(stmt->body, scope);
            if (res->type == ValueType::ReturnSignal) break;

        } else break;
    }

    return res;
}