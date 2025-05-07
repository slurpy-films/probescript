#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "body.hpp"

Val evalIfStmt(IfStmtType* stmt, Env* baseEnv) {
    Val condition = eval(stmt->condition, baseEnv);

    if (condition->type != ValueType::Boolean) {
        std::cerr << "If statement condition must evaluate to a boolean, got " << condition->value;
        exit(1);
    }

    std::shared_ptr<BooleanVal> boolval = std::static_pointer_cast<BooleanVal>(condition);

    if (boolval->getValue()) {
        Env* env = new Env(baseEnv);
        return evalBody(stmt->body, env);
    } else if (stmt->hasElse) {
        Env* env = new Env(baseEnv);
        return evalBody(stmt->elseStmt, env);
    }

    return std::make_shared<UndefinedVal>();
}
