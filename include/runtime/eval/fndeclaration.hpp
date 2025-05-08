#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

Val evalFunctionDeclaration(FunctionDeclarationType* declaration, Env* env, bool onlyValue = false) {
    std::shared_ptr<FunctionValue> fn = std::make_shared<FunctionValue>(declaration->name, declaration->parameters, env, declaration->body);

    return onlyValue ? fn : env->declareVar(declaration->name, fn, true);
}