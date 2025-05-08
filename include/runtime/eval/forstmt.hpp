#pragma once
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "ast.hpp"
#include "body.hpp"

Val evalForStmt(ForStmtType* forstmt, Env* env) {
    Env* parent = new Env(env);

    for (Stmt* stmt : forstmt->declarations) {
        eval(stmt, parent);
    }

    while (true) {
        Env* scope = new Env(parent);
        std::vector<Val> conds;
        for (Expr* expr : forstmt->conditions) {
            conds.push_back(eval(expr, scope));
        }

        bool breaking = false;

        for (Val cond : conds) {
            if (cond->type != ValueType::Boolean) {
                std::cerr << "For loop condition must evaluate to a boolean";
                exit(1);
            }

            if (!std::static_pointer_cast<BooleanVal>(cond)->toBool()) {
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

    return std::make_shared<UndefinedVal>();
}