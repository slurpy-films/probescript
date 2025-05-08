#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "body.hpp"

Val evalProbeCall(std::string probeName, Env* declarationEnv, std::vector<Val> args);

#include "runprobe.hpp"

Val evalCall(CallExprType* call, Env* env) {
    std::vector<Val> args;

    for (Expr* arg : call->args) {
        args.push_back(eval(arg, env));
    };

    Val fn = eval(call->calee, env);

    if (fn == nullptr) {
        std::cerr << "Error: function call target is null";
        exit(1);
    }

    if (fn->type == ValueType::NativeFn) {

        Val result = std::static_pointer_cast<NativeFnValue>(fn)->call(args, env);

        return result;
    }

    if (fn->type == ValueType::Function) {
        std::shared_ptr<FunctionValue> func = std::static_pointer_cast<FunctionValue>(fn);
        Env* scope = new Env(func->declarationEnv);

        for (int i = 0; i < func->params.size(); i++) {
            std::string varname = func->params[i];
            scope->declareVar(varname, i < args.size() ? args[i]: std::make_shared<UndefinedVal>(), false);
        }

        Val result = evalBody(func->body, scope);

        return result->type == ValueType::ReturnSignal ? std::static_pointer_cast<ReturnSignal>(result)->val : std::make_shared<UndefinedVal>();
    }

    if (fn->type == ValueType::Probe) {
        return evalProbeCall(std::static_pointer_cast<ProbeValue>(fn)->name, env, args);
    }

    std::cerr << "Cannot call value that is not a function, " << fn->type;
    exit(1);
}

Val evalCallWithFnVal(Val fn, std::vector<Val> args, Env* env) {

    if (fn == nullptr) {
        std::cerr << "Error: function call target is null";
        exit(1);
    }

    if (fn->type == ValueType::NativeFn) {

        Val result = std::static_pointer_cast<NativeFnValue>(fn)->call(args, env);

        return result;
    }

    if (fn->type == ValueType::Function) {
        std::shared_ptr<FunctionValue> func = std::static_pointer_cast<FunctionValue>(fn);
        Env* scope = new Env(func->declarationEnv);

        for (int i = 0; i < func->params.size(); i++) {
            std::string varname = func->params[i];
            Val value = (i < args.size()) ? args[i] : std::make_shared<UndefinedVal>();
            scope->declareVar(varname, value, false);
        }
        
        Val result = std::make_shared<UndefinedVal>();

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
        return evalProbeCall(std::static_pointer_cast<ProbeValue>(fn)->name, env, args);
    }

    std::cerr << "Cannot call value that is not a function, " << fn->type;
    exit(1);
}