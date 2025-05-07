#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

std::shared_ptr<StringVal> evalStringericBinExpr(std::shared_ptr<StringVal> lhs, std::shared_ptr<StringVal> rhs, std::string op) {
    std::string result = "";

    std::string left = lhs->string;
    std::string right = rhs->string;

    if (op == "+") {
        result = left + right;
    }

    return std::make_shared<StringVal>(result);
}