#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalProbeCall(string probeName, Env* declarationEnv);

#include "runprobe.hpp"

RuntimeVal* evalCall(CallExprType* call, Env* env) {
    vector<RuntimeVal*> args;

    for (auto arg : call->args) {
        args.push_back(eval(arg, env));
    };

    RuntimeVal* fn = eval(call->calee, env);

    if (fn == nullptr) {
        cerr << "Error: function call target is null";
        exit(1);
    }

    if (fn->type == ValueType::NativeFn) {

        RuntimeVal* result = static_cast<NativeFnValue*>(fn)->call(args, env);

        return result;
    }

    if (fn->type == ValueType::Function) {
        FunctionValue* func = static_cast<FunctionValue*>(fn);
        Env* scope = new Env(func->declarationEnv);

        for (int i = 0; i < func->params.size(); i++) {
            string varname = func->params[i];
            scope->declareVar(varname, args[i], false);
        }

        RuntimeVal* result = new UndefinedVal();

        for (Stmt* stmt : func->body) {
            result = eval(stmt, scope);
        }

        return result;
    }

    if (fn->type == ValueType::Probe) {
        return evalProbeCall(static_cast<ProbeValue*>(fn)->name, env);
    }

    cerr << "Cannot call value that is not a function, " << fn->type;
    exit(1);
}