#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "ast.hpp"
#include "body.hpp"

RuntimeVal* evalForStmt(ForStmtType* forstmt, Env* env) {
    Env* scope = new Env(env);

    for (Stmt* stmt : forstmt->declarations) {
        eval(stmt, scope);
    }

    while (true) {
        std::vector<RuntimeVal*> conds;
        for (Expr* expr : forstmt->conditions) {
            conds.push_back(eval(expr, scope));
        }

        bool breaking = false;

        for (RuntimeVal* cond : conds) {
            if (cond->type != ValueType::Boolean) {
                std::cerr << "For loop condition must evaluate to a boolean";
                exit(1);
            }

            if (!static_cast<BooleanVal*>(cond)->toBool()) {
                breaking = true;
                break;
            }
        }

        if (breaking) break;

        evalBody(forstmt->body, scope);

        for (Expr* expr : forstmt->updates) {
            eval(expr, scope);
        }
    }

    delete scope;

    return new UndefinedVal();
}