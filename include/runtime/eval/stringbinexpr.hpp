#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

StringVal* evalStringericBinExpr(StringVal* lhs, StringVal* rhs, std::string op) {
    std::string result = "";

    std::string left = lhs->string;
    std::string right = rhs->string;

    if (op == "+") {
        result = left + right;
    }

    return new StringVal(result);
}