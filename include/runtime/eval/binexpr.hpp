#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "boolbinop.hpp"

static std::unordered_map<std::string, bool> booleanOperators = {{"&&", true}, {"||", true}, {">=", true}, {"<=", true}, {"<", true}, {">", true}, {"!=", true}, {"==", true}};

RuntimeVal* evalBinExpr(BinaryExprType* binop, Env* env) {
    if (booleanOperators.find(binop->op) != booleanOperators.end()) {
        return evalBooleanBinExpr(binop, env);
    }

    RuntimeVal* left = eval(binop->left, env);
    RuntimeVal* right = eval(binop->right, env);

    if (left->type == ValueType::Number && right->type == ValueType::Number) {
        return evalNumericBinExpr(static_cast<NumberVal*>(left), static_cast<NumberVal*>(right), binop->op);
    }

    if (left->type == ValueType::String && right->type == ValueType::String) {
        return evalStringericBinExpr(static_cast<StringVal*>(left), static_cast<StringVal*>(right), binop->op);
    }

    std::cerr << "Invalid operants: " << left->value << " and " << right->value;
    exit(1);
}