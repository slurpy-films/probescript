#include "runtime/interpreter.hpp"

using namespace Probescript;
using namespace Probescript::Interpreter;

Values::Val Interpreter::evalCall(std::shared_ptr<AST::CallExprType> call, EnvPtr env)
{
    std::vector<Values::Val> args;

    for (std::shared_ptr<AST::Expr> arg : call->args)
    {
        args.push_back(eval(arg, env));
    };

    Values::Val fn = eval(call->calee, env);

    return evalCallWithFnVal(fn, args, env);
}

Values::Val Interpreter::evalCallWithFnVal(Values::Val fn, std::vector<Values::Val> args, EnvPtr env)
{

    if (fn == nullptr)
    {
        throw ThrowException(CustomError("Function call target is null", "FunctionCallError", fn->token));
    }

    if (fn->type == Values::ValueType::NativeFn)
    {
        auto nativefn = std::static_pointer_cast<Values::NativeFnValue>(fn);

        Values::Val result = nativefn->call(args, env);

        return result;
    }

    if (fn->type == Values::ValueType::Function)
    {
        std::shared_ptr<Values::FunctionValue> func = std::static_pointer_cast<Values::FunctionValue>(fn);
        
        if (func->isAsync)
        {
            return std::make_shared<Values::FutureVal>(std::async(std::launch::async, [func, env, args]() -> Values::Val
            {
                EnvPtr scope = std::make_shared<Env>(func->declarationEnv);

                for (int i = 0; i < func->params.size(); i++)
                {
                    std::string varname = func->params[i]->identifier;
                    Values::Val value = (i < args.size()) ? args[i] : eval(func->params[i]->value, env);
                    scope->declareVar(varname, value, Lexer::Token());
                }

                Values::Val result = std::make_shared<Values::UndefinedVal>();

                try
                {
                    evalBody(func->body, scope);
                }
                catch (const Values::ReturnSignal& ret)
                {
                    result = ret.get(); 
                }
                
                return result;
            }));
        }
        else
        {
            EnvPtr scope = std::make_shared<Env>(func->declarationEnv);

            for (int i = 0; i < func->params.size(); i++)
            {
                std::string varname = func->params[i]->identifier;
                Values::Val value = (i < args.size()) ? args[i] : eval(func->params[i]->value, env);
                scope->declareVar(varname, value, fn->token);
            }
            Values::Val result = std::make_shared<Values::UndefinedVal>();

            try
            {
                evalBody(func->body, scope);
            }
            catch (const Values::ReturnSignal& ret)
            {
                result = ret.get(); 
            }
            
            return result;
        }
    }

    if (fn->type == Values::ValueType::Probe)
    {
        return evalProbeCall(fn, env, args);
    }

    throw ThrowException(CustomError("Cannot call value that is not a function", "FunctionCallError"));
}

Values::Val Interpreter::evalAwaitExpr(std::shared_ptr<AST::AwaitExprType> expr, EnvPtr env) {
    Values::Val result = eval(expr->caller, env);
    
    if (result->type != Values::ValueType::Future) {
        throw ThrowException(ArgumentError("'await' requires a future"));
    }
    
    std::shared_ptr<Values::FutureVal> future = std::static_pointer_cast<Values::FutureVal>(result);
    
    try {
        return future->future.get();
    } catch (...) {
        throw ThrowException(CustomError("Async function failed", "AsyncError"));
    }
}