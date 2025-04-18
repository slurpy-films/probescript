#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

StringVal* evalStringericBinExpr(StringVal* lhs, StringVal* rhs, string op) {
    string result = "";

    string left = lhs->string;
    string right = rhs->string;

    if (op == "+") {
        result = left + right;
    }

    return new StringVal(result);
}