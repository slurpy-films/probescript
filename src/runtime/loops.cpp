#include "runtime/interpreter.hpp"

Val evalForStmt(ForStmtType* forstmt, Env* env) {
    Env* parent = new Env(env);

    for (Stmt* stmt : forstmt->declarations) {
        eval(stmt, parent);
    }

    Val result = std::make_shared<UndefinedVal>();

    while (true) {
        Env* scope = new Env(parent);
        std::vector<Val> conds;
        for (Expr* expr : forstmt->conditions) {
            conds.push_back(eval(expr, scope));
        }

        bool breaking = false;  

        for (Val cond : conds) {
            if (cond->type != ValueType::Boolean) {
                return env->throwErr(ManualError("For loop condition must evaluate to a boolean", "ForLoopError"));
            }

            if (!std::static_pointer_cast<BooleanVal>(cond)->toBool()) {
                breaking = true;
                break;
            }
        }

        if (breaking) break;

        result = evalBody(forstmt->body, scope, true);
        if (result->type == ValueType::ReturnSignal) break;
        else if (result->type == ValueType::BreakSignal) break;

        for (Expr* expr : forstmt->updates) {
            eval(expr, scope);
        }
    }

    return result->type == ValueType::ReturnSignal ? result : std::make_shared<UndefinedVal>();
}

Val evalWhileStmt(WhileStmtType* stmt, Env* env) {
    Val res = std::make_shared<UndefinedVal>();
    while (true) {
        Val result = eval(stmt->condition, env);
        if (result->type != ValueType::Boolean) {
            return env->throwErr(ManualError("While condition must evaluate to a boolean", "WhileError"));
        }

        if (std::static_pointer_cast<BooleanVal>(result)->getValue()) {
            Env* scope = new Env(env);
            res = evalBody(stmt->body, scope, true);
            if (res->type == ValueType::ReturnSignal) break;
            else if (res->type == ValueType::BreakSignal) break;

        } else break;
    }

    return res;
}