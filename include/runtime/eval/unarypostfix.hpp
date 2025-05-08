#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include <memory>

std::shared_ptr<RuntimeVal> evalUnaryPostFix(UnaryPostFixType* expr, Env* env) {
    auto current = eval(expr->assigne, env);
    std::string varName;

    if (expr->assigne->kind == NodeType::Identifier) {
        varName = static_cast<IdentifierType*>(expr->assigne)->symbol;
    } else {
        std::cerr << "Cannot use postfix operator on non-identifier";
        exit(1);
    }

    if (current->type != ValueType::Number) {
        std::cerr << "Cannot use postfix operator on non-number";
        exit(1);
    }

    std::shared_ptr<NumberVal> numberVal = std::static_pointer_cast<NumberVal>(current);
    double value = numberVal->toNum();
    double newValue;

    if (expr->op == "++") {
        newValue = value + 1;
    } else if (expr->op == "--") {
        newValue = value - 1;
    } else {
        std::cerr << "Unknown postfix operator: " << expr->op;
        exit(1);
    }

    env->assignVar(varName, std::make_shared<NumberVal>(newValue));
    return std::make_shared<NumberVal>(value);
} 