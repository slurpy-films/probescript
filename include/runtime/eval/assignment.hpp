#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "runtime/interpreter.hpp"
#include "memberassignment.hpp"

Val evalAssignment(AssignmentExprType* assignment, Env* env) {
    if (assignment->assigne->kind != NodeType::Identifier) {
        std::cerr << "Expected Identifier in assignment" << std::endl;
        exit(1);
    }

    std::string varName = static_cast<IdentifierType*>(assignment->assigne)->symbol;

    Val leftVal = eval(assignment->assigne, env);
    Val rightVal = eval(assignment->value, env);

    if (assignment->op == "=") {
        return env->assignVar(varName, rightVal);
    }

    if (leftVal->type != ValueType::Number || rightVal->type != ValueType::Number) {
        std::cerr << "Assignment operator '" << assignment->op << "' requires numeric values." << std::endl;
        exit(1);
    }

    double left = std::static_pointer_cast<NumberVal>(leftVal)->toNum();
    double right = std::static_pointer_cast<NumberVal>(rightVal)->toNum();
    double result;

    if (assignment->op == "+=") result = left + right;
    else if (assignment->op == "-=") result = left - right;
    else if (assignment->op == "*=") result = left * right;
    else if (assignment->op == "/=") result = left / right;
    else {
        std::cerr << "Unsupported assignment operator: " << assignment->op << std::endl;
        exit(1);
    }

    return env->assignVar(varName, std::make_shared<NumberVal>(result));
}

Val evalUnaryPostfix(UnaryPostFixType* expr, Env* env) {
    if (expr->assigne->kind == NodeType::Identifier) {
        std::string varName = static_cast<IdentifierType*>(expr->assigne)->symbol;
        Val current = env->lookupVar(varName);

        if (current->type != ValueType::Number) {
            std::cerr << "Postfix operators only supported on numbers" << std::endl;
            exit(1);
        }

        double value = std::static_pointer_cast<NumberVal>(current)->toNum();
        double newValue = value;

        if (expr->op == "++") newValue = value + 1;
        else if (expr->op == "--") newValue = value - 1;
        else {
            std::cerr << "Unknown postfix operator: " << expr->op << std::endl;
            exit(1);
        }

        env->assignVar(varName, std::make_shared<NumberVal>(newValue));

        return std::make_shared<NumberVal>(value);
    } else if (expr->assigne->kind == NodeType::MemberExpr) {
        MemberAssignmentType* member = new MemberAssignmentType(
            static_cast<MemberExprType*>(expr->assigne)->object,
            static_cast<MemberExprType*>(expr->assigne)->property,
            new NumericLiteralType((expr->op == "++") ? 1 : -1),
            static_cast<MemberExprType*>(expr->assigne)->computed
        );

        return evalMemberAssignment(member, env);
    }

    return std::make_shared<UndefinedVal>();
}


Val evalUnaryPrefix(UnaryPrefixType* expr, Env* env) {
    Val val = eval(expr->assigne, env);

    if (expr->op == "!") {
        return std::make_shared<BooleanVal>(!val->toBool());
    }


    return std::make_shared<UndefinedVal>();
}