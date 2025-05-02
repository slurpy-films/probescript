#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

NumberVal* evalNumericBinExpr(NumberVal* lhs, NumberVal* rhs, std::string op) {
    double result = 0;

    double left = std::stod(lhs->value);
    double right = std::stod(rhs->value);
    
    if (op == "+") {
        result = left + right;
    } else if (op == "-") {
        result = left - right;
    } else if (op == "*") {
        result = left * right;
    } else if (op == "/") {
        result = left / right;
    } else if (op == "%") {
        result = fmod(left, right);
    }

    return new NumberVal(std::to_string(result));
}