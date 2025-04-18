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

    return env->assignVar(varName, eval(assignment->value, env));
}