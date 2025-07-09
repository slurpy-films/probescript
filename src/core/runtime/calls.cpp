#include "runtime/interpreter.hpp"

Val evalCall(std::shared_ptr<CallExprType> call, EnvPtr env)
{
    std::vector<Val> args;

    for (std::shared_ptr<Expr> arg : call->args)
    {
        args.push_back(eval(arg, env));
    };

    Val fn = eval(call->calee, env);

    return evalCallWithFnVal(fn, args, env);
}

Val evalCallWithFnVal(Val fn, std::vector<Val> args, EnvPtr env)
{

    if (fn == nullptr)
    {
        throw ThrowException(CustomError("Function call target is null", "FunctionCallError", fn->token));
    }

    if (fn->type == ValueType::NativeFn)
    {
        auto nativefn = std::static_pointer_cast<NativeFnValue>(fn);

        Val result = nativefn->call(args, env);

        return result;
    }

    if (fn->type == ValueType::Function)
    {
        std::shared_ptr<FunctionValue> func = std::static_pointer_cast<FunctionValue>(fn);
        
        if (func->isAsync)
        {
            return std::make_shared<FutureVal>(std::async(std::launch::async, [func, env, args]() -> Val
            {
                EnvPtr scope = std::make_shared<Env>(func->declarationEnv);

                for (int i = 0; i < func->params.size(); i++)
                {
                    std::string varname = func->params[i]->identifier;
                    Val value = (i < args.size()) ? args[i] : eval(func->params[i]->value, env);
                    scope->declareVar(varname, value, Lexer::Token());
                }

                Val result = std::make_shared<UndefinedVal>();

                try
                {
                    evalBody(func->body, scope);
                }
                catch (const ReturnSignal& ret)
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
                Val value = (i < args.size()) ? args[i] : eval(func->params[i]->value, env);
                scope->declareVar(varname, value, fn->token);
            }
            Val result = std::make_shared<UndefinedVal>();

            try
            {
                evalBody(func->body, scope);
            }
            catch (const ReturnSignal& ret)
            {
                result = ret.get(); 
            }
            
            return result;
        }
    }

    if (fn->type == ValueType::Probe)
    {
        return evalProbeCall(fn, env, args);
    }

    throw ThrowException(CustomError("Cannot call value that is not a function", "FunctionCallError"));
}

Val evalAwaitExpr(std::shared_ptr<AwaitExprType> expr, EnvPtr env) {
    Val result = eval(expr->caller, env);
    
    if (result->type != ValueType::Future) {
        throw ThrowException(ArgumentError("'await' requires a future"));
    }
    
    std::shared_ptr<FutureVal> future = std::static_pointer_cast<FutureVal>(result);
    
    try {
        return future->future.get();
    } catch (...) {
        throw ThrowException(CustomError("Async function failed", "AsyncError"));
    }
}