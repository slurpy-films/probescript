#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

std::shared_ptr<NumberVal> evalNumericBinExpr(std::shared_ptr<NumberVal> lhs, std::shared_ptr<NumberVal> rhs, std::string op) {
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

    return std::make_shared<NumberVal>(result);
}