#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalAssignment(AssignmentExprType* assignment, Env* env) {
    if (assignment->assigne->kind != NodeType::Identifier) {
        cout << "Expected Identifier" << endl;
        exit(1);
    }

    string varName = static_cast<IdentifierType*>(assignment->assigne)->symbol;

    if (assignment->op == "=") {
        return env->assignVar(varName, eval(assignment->value, env));
    } else if (assignment->op == "+=") {
        RuntimeVal* val = eval(assignment->assigne, env);

        switch (val->type) {
            case ValueType::Number:
                return env->assignVar(varName, new NumberVal(to_string(static_cast<NumberVal*>(val)->toNum() + eval(assignment->value, env)->toNum())));
            case ValueType::String:
                return env->assignVar(varName, new StringVal(static_cast<StringVal*>(val)->toString() + eval(assignment->value, env)->toString()));
            default:
                cerr << "Cannot only use += operator on strings and numbers";
                exit(1);
        }
    } else if (assignment->op == "-=") {
        RuntimeVal* val = eval(assignment->assigne, env);

        switch (val->type) {
            case ValueType::Number:
                return env->assignVar(varName, new NumberVal(to_string(static_cast<NumberVal*>(val)->toNum() - eval(assignment->value, env)->toNum())));
            default:
                cerr << "Cannot only use -= operator on numbers";
                exit(1);
        }
    }

    return new UndefinedVal();
    
}