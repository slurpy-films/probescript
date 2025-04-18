#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

RuntimeVal* evalIdent(IdentifierType* ident, Env* env) {
    RuntimeVal* value = env->lookupVar(ident->symbol);
    return value;
}