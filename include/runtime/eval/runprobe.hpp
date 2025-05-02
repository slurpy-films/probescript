#pragma once
#include <string>
#include "ast.hpp"
#include "call.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

RuntimeVal* evalProbeCall(std::string probeName, Env* declarationEnv) {
    RuntimeVal* val = declarationEnv->lookupVar(probeName);

    if (val->type != ValueType::Probe) {
        std::cerr << "Probe " << probeName << " is not of type probe";
        exit(1);
    }

    ProbeValue* probe = static_cast<ProbeValue*>(val);

    Env* env = new Env(declarationEnv);

    for (Stmt* stmt : probe->body) {
        switch (stmt->kind) {
            case NodeType::VarDeclaration:
                eval(stmt, env);
                break;
            case NodeType::FunctionDeclaration:
                eval(stmt, env);
                break;
            default:
                std::cerr << "Only variable and function declarations are allowed within probe bodies";
                exit(1);
        }
    }

    RuntimeVal* last = new UndefinedVal();

    RuntimeVal* runfnval = env->lookupVar("run");

    if (runfnval->type != ValueType::Function) {
        std::cerr << "Expected run to be of type function, got " << runfnval->type;
        exit(1);
    }

    FunctionValue* runfn = static_cast<FunctionValue*>(runfnval);

    for (Stmt* stmt : runfn->body) {
        last = eval(stmt, env);
    }

    return last;
}