#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"

Val evalVarDeclaration(VarDeclarationType* var, Env* env, bool constant = false) {
    Val value = var->value != nullptr ? eval(var->value, env) : std::make_shared<UndefinedVal>();
    return env->declareVar(var->identifier, value, constant);
}