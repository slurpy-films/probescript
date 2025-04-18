#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalFunctionDeclaration(FunctionDeclarationType* declaration, Env* env) {
    FunctionValue* fn = new FunctionValue(declaration->name, declaration->parameters, env, declaration->body);

    return env->declareVar(declaration->name, fn, true);
}