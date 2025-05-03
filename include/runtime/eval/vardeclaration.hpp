#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalVarDeclaration(VarDeclarationType* var, Env* env, bool constant = false) {
    RuntimeVal* value = var->value != nullptr ? eval(var->value, env) : new UndefinedVal();
    return env->declareVar(var->identifier, value, constant);
}