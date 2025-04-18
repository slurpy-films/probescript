#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

RuntimeVal* evalBinExpr(BinaryExprType* binop, Env* env) {
    RuntimeVal* left = eval(binop->left, env);
    RuntimeVal* right = eval(binop->right, env);

    if (left->type == ValueType::Number && right->type == ValueType::Number) {
        return evalNumericBinExpr(static_cast<NumberVal*>(left), static_cast<NumberVal*>(right), binop->op);
    }

    if (left->type == ValueType::String && right->type == ValueType::String) {
        return evalStringericBinExpr(static_cast<StringVal*>(left), static_cast<StringVal*>(right), binop->op);
    }

    cerr << "Invalid operants: " << left->value << " and " << right->value;
    exit(1);
}