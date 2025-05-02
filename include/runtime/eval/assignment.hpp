#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalAssignment(AssignmentExprType* assignment, Env* env) {
    if (assignment->assigne->kind != NodeType::Identifier) {
        std::cout << "Expected Identifier" << std::endl;
        exit(1);
    }

    std::string varName = static_cast<IdentifierType*>(assignment->assigne)->symbol;

    if (assignment->op == "=") {
        return env->assignVar(varName, eval(assignment->value, env));
    } else if (assignment->op == "+=") {
        RuntimeVal* val = eval(assignment->assigne, env);

        switch (val->type) {
            case ValueType::Number:
                return env->assignVar(varName, new NumberVal(std::to_string(static_cast<NumberVal*>(val)->toNum() + eval(assignment->value, env)->toNum())));
            case ValueType::String:
                return env->assignVar(varName, new StringVal(static_cast<StringVal*>(val)->toString() + eval(assignment->value, env)->toString()));
            default:
                std::cerr << "Cannot only use += operator on strings and numbers";
                exit(1);
        }
    } else if (assignment->op == "-=") {
        RuntimeVal* val = eval(assignment->assigne, env);

        switch (val->type) {
            case ValueType::Number:
                return env->assignVar(varName, new NumberVal(std::to_string(static_cast<NumberVal*>(val)->toNum() - eval(assignment->value, env)->toNum())));
            default:
            std::cerr << "Cannot only use -= operator on numbers";
                exit(1);
        }
    }

    return new UndefinedVal();
}