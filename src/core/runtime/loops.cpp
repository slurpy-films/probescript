#include "runtime/interpreter.hpp"

Val evalForStmt(ForStmtType* forstmt, EnvPtr env)
{
    EnvPtr parent = std::make_shared<Env>(env);

    for (Stmt* stmt : forstmt->declarations)
    {
        eval(stmt, parent);
    }

    Val result = std::make_shared<UndefinedVal>();

    while (true)
    {
        EnvPtr scope = std::make_shared<Env>(parent);
        std::vector<Val> conds;
        for (Expr* expr : forstmt->conditions)
        {
            conds.push_back(eval(expr, scope));
        }

        bool breaking = false;  

        for (Val cond : conds)
        {
            if (!cond->toBool())
            {
                breaking = true;
                break;
            }
        }

        if (breaking) break;
        try
        {
            evalBody(forstmt->body, scope, true);
        }
        catch (const BreakSignal& signal)
        {
            break;
        }
        catch (const ContinueSignal& signal)
        {
            for (Expr* expr : forstmt->updates)
            {
                eval(expr, scope);
            }
            continue;
        }

        for (Expr* expr : forstmt->updates)
        {
            eval(expr, scope);
        }
    }

    return std::make_shared<UndefinedVal>();
}

Val evalWhileStmt(WhileStmtType* stmt, EnvPtr env) {
    while (true) {
        Val result = eval(stmt->condition, env);

        if (result->toBool()) {
            EnvPtr scope = std::make_shared<Env>(env);
            try
            {
                evalBody(stmt->body, scope);
            }
            catch (const BreakSignal& signal)
            {
                break;
            }
            catch (const ContinueSignal& signal)
            {
                continue;
            }
        } else break;
    }

    return std::make_shared<UndefinedVal>();
}