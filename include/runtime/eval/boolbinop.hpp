#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
Val evalBooleanBinExpr(BinaryExprType* binop, Env* env) {
    Val left = eval(binop->left, env);
    Val right = eval(binop->right, env);

    const std::string& op = binop->op;

    if (op == "&&" || op == "||") {
        bool l = left->toBool();
        bool r = right->toBool();
        return std::make_shared<BooleanVal>((op == "&&") ? (l && r) : (l || r));
    }

    if (op == "==" || op == "!=") {
        bool result = (left->toString() == right->toString());
        return std::make_shared<BooleanVal>(op == "==" ? result : !result);
    }

    if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        double l = left->toNum();
        double r = right->toNum();
        bool result = false;

        if (op == "<") result = l < r;
        else if (op == ">") result = l > r;
        else if (op == "<=") result = l <= r;
        else if (op == ">=") result = l >= r;

        return std::make_shared<BooleanVal>(result);
    }

    std::cerr << "Invalid binary boolean operator: " << op << "\n";
    exit(1);
}
