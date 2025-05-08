#pragma once
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "ast.hpp"

Val evalArrowFunction(ArrowFunctionType* fn, Env* env) {
    return std::make_shared<FunctionValue>("arrow", fn->params, env, fn->body);
}