#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalProbeCall(std::string probeName, Env* declarationEnv);

#include "runprobe.hpp"

RuntimeVal* evalCall(CallExprType* call, Env* env) {
    std::vector<RuntimeVal*> args;

    for (Expr* arg : call->args) {
        args.push_back(eval(arg, env));
    };

    RuntimeVal* fn = eval(call->calee, env);

    if (fn == nullptr) {
        std::cerr << "Error: function call target is null";
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
            std::string varname = func->params[i];
            scope->declareVar(varname, args[i], false);
        }

        RuntimeVal* result = new UndefinedVal();

        for (Stmt* stmt : func->body) {
            if (stmt->kind == NodeType::ReturnStmt) {
                result = eval(static_cast<ReturnStmtType*>(stmt)->stmt, scope);
            } else {
                result = eval(stmt, scope);
            }
        }

        return result;
    }

    if (fn->type == ValueType::Probe) {
        return evalProbeCall(static_cast<ProbeValue*>(fn)->name, env);
    }

    std::cerr << "Cannot call value that is not a function, " << fn->type;
    exit(1);
}

RuntimeVal* evalCallWithFnVal(RuntimeVal* fn, std::vector<RuntimeVal*> args, Env* env) {

    if (fn == nullptr) {
        std::cerr << "Error: function call target is null";
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
            std::string varname = func->params[i];
            RuntimeVal* value = (i < args.size()) ? args[i] : new UndefinedVal();
            scope->declareVar(varname, value, false);
        }
        
        RuntimeVal* result = new UndefinedVal();

        for (Stmt* stmt : func->body) {
            if (stmt->kind == NodeType::ReturnStmt) {
                result = eval(static_cast<ReturnStmtType*>(stmt)->stmt, scope);
            } else {
                result = eval(stmt, scope);
            }
        }

        return result;
    }

    if (fn->type == ValueType::Probe) {
        return evalProbeCall(static_cast<ProbeValue*>(fn)->name, env);
    }

    std::cerr << "Cannot call value that is not a function, " << fn->type;
    exit(1);
}