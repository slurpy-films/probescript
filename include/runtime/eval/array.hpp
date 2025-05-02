#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/env.hpp"

RuntimeVal* evalArray(ArrayLiteralType* expr, Env* env) {
    std::vector<RuntimeVal*> items;

    for (Expr* item : expr->items) {
        items.push_back(eval(item, env));
    }

    return new ArrayVal(items);
}