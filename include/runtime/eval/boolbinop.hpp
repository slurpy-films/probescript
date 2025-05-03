#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

RuntimeVal* evalBooleanBinExpr(BinaryExprType* binop, Env* env) {
    RuntimeVal* left = eval(binop->left, env);
    RuntimeVal* right = eval(binop->right, env);

    const std::string& op = binop->op;

    if (op == "&&" || op == "||") {
        bool l = left->toBool();
        bool r = right->toBool();
        return new BooleanVal((op == "&&") ? (l && r) : (l || r));
    }

    if (op == "==" || op == "!=") {
        bool result = (left->toString() == right->toString());
        return new BooleanVal(op == "==" ? result : !result);
    }

    if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        double l = left->toNum();
        double r = right->toNum();
        bool result = false;

        if (op == "<") result = l < r;
        else if (op == ">") result = l > r;
        else if (op == "<=") result = l <= r;
        else if (op == ">=") result = l >= r;

        return new BooleanVal(result);
    }

    std::cerr << "Invalid binary boolean operator: " << op << "\n";
    exit(1);
}
