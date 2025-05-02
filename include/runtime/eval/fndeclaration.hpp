#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalFunctionDeclaration(FunctionDeclarationType* declaration, Env* env, bool onlyValue = false) {
    FunctionValue* fn = new FunctionValue(declaration->name, declaration->parameters, env, declaration->body);

    return onlyValue ? fn : env->declareVar(declaration->name, fn, true);
}