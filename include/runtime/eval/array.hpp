#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/env.hpp"

Val evalArray(ArrayLiteralType* expr, Env* env) {
    std::vector<Val> items;

    for (Expr* item : expr->items) {
        items.push_back(eval(item, env));
    }

    return std::make_shared<ArrayVal>(items);
}