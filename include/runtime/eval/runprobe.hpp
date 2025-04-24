#pragma once
#include <string>
#include "ast.hpp"
#include "call.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

RuntimeVal* evalProbeCall(string probeName, Env* declarationEnv) {
    RuntimeVal* val = declarationEnv->lookupVar(probeName);

    if (val->type != ValueType::Probe) {
        cerr << "Probe " << probeName << " is not of type probe";
        exit(1);
    }

    ProbeValue* probe = static_cast<ProbeValue*>(val);

    Env* env = new Env(declarationEnv);

    for (Stmt* stmt : probe->body) {
        eval(stmt, env);
    }

    RuntimeVal* last = new UndefinedVal();

    RuntimeVal* runfnval = env->lookupVar("run");

    if (runfnval->type != ValueType::Function) {
        cerr << "Expected run to be of type function, got " << runfnval->type;
        exit(1);
    }

    FunctionValue* runfn = static_cast<FunctionValue*>(runfnval);

    for (Stmt* stmt : runfn->body) {
        last = eval(stmt, env);
    }

    return last;
}