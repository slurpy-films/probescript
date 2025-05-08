#pragma once
#include <string>
#include "ast.hpp"
#include "call.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

Val evalProbeCall(std::string probeName, Env* declarationEnv, std::vector<Val> args = {}) {
    Val val = declarationEnv->lookupVar(probeName);

    if (val->type != ValueType::Probe) {
        std::cerr << "Probe " << probeName << " is not of type probe";
        exit(1);
    }

    std::shared_ptr<ProbeValue> probe = std::static_pointer_cast<ProbeValue>(val);

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


    Val runfnval = env->lookupVar("run");

    if (runfnval->type != ValueType::Function) {
        std::cerr << "Expected run to be of type function, got " << runfnval->type;
        exit(1);
    }


    evalCallWithFnVal(runfnval, args, env);

    return std::make_shared<UndefinedVal>();
}