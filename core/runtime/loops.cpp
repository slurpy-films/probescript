#include "runtime/interpreter.hpp"

using namespace Probescript;
using namespace Probescript::Interpreter;

Values::Val Interpreter::evalForStmt(std::shared_ptr<AST::ForStmtType> forstmt, EnvPtr env)
{
    EnvPtr parent = std::make_shared<Env>(env);

    for (std::shared_ptr<AST::Stmt> stmt : forstmt->declarations)
    {
        eval(stmt, parent);
    }

    Values::Val result = std::make_shared<Values::UndefinedVal>();

    while (true)
    {
        EnvPtr scope = std::make_shared<Env>(parent);

        bool breaking = false;
        
        for (std::shared_ptr<AST::Expr> expr : forstmt->conditions)
        {
            if (!eval(expr, scope)->toBool())
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
        catch (const Values::BreakSignal& signal)
        {
            break;
        }
        catch (const Values::ContinueSignal& signal)
        {
            for (std::shared_ptr<AST::Expr> expr : forstmt->updates)
            {
                eval(expr, scope);
            }
            continue;
        }

        for (std::shared_ptr<AST::Expr> expr : forstmt->updates)
        {
            eval(expr, scope);
        }
    }

    return std::make_shared<Values::UndefinedVal>();
}

Values::Val Interpreter::evalWhileStmt(std::shared_ptr<AST::WhileStmtType> stmt, EnvPtr env) {
    while (true) {
        Values::Val result = eval(stmt->condition, env);

        if (result->toBool()) {
            EnvPtr scope = std::make_shared<Env>(env);
            try
            {
                evalBody(stmt->body, scope);
            }
            catch (const Values::BreakSignal& signal)
            {
                break;
            }
            catch (const Values::ContinueSignal& signal)
            {
                continue;
            }
        } else break;
    }

    return std::make_shared<Values::UndefinedVal>();
}