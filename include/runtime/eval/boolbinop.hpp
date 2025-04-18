#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

RuntimeVal* evalBooleanBinExpr(BinaryExprType* binop, Env* env) {
    RuntimeVal* left = eval(binop->left, env);
    RuntimeVal* right = eval(binop->right, env);

    if (binop->op == "&&" || binop->op == "||") {
        bool boolean = binop->op == "&&" ? static_cast<BooleanVal*>(left)->getValue() && static_cast<BooleanVal*>(right)->getValue() : static_cast<BooleanVal*>(left)->getValue() || static_cast<BooleanVal*>(right)->getValue();
        return new BooleanVal(to_string(boolean));
    }

    cout << "Invalid operants";

    exit(1);
}