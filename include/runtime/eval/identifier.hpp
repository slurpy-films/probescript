#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

Val evalIdent(IdentifierType* ident, Env* env) {
    Val value = env->lookupVar(ident->symbol);
    return value;
}