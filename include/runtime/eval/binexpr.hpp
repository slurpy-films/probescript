#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "boolbinop.hpp"

static std::unordered_map<std::string, bool> booleanOperators = {{"&&", true}, {"||", true}, {">=", true}, {"<=", true}, {"<", true}, {">", true}, {"!=", true}, {"==", true}};

Val evalBinExpr(BinaryExprType* binop, Env* env) {
    if (booleanOperators.find(binop->op) != booleanOperators.end()) {
        return evalBooleanBinExpr(binop, env);
    }

    Val left = eval(binop->left, env);
    Val right = eval(binop->right, env);

    if (left->type == ValueType::Number && right->type == ValueType::Number) {
        return evalNumericBinExpr(std::static_pointer_cast<NumberVal>(left), std::static_pointer_cast<NumberVal>(right), binop->op);
    }

    if (left->type == ValueType::String && right->type == ValueType::String) {
        return evalStringericBinExpr(std::static_pointer_cast<StringVal>(left), std::static_pointer_cast<StringVal>(right), binop->op);
    }

    std::cerr << "Invalid operants: " << left->value << " and " << right->value;
    exit(1);
}