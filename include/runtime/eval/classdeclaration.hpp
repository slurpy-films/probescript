#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "runtime/interpreter.hpp"

Val evalClassDefinition(ClassDefinitionType* def, Env* env) {
    return env->declareVar(def->name, def->doesExtend ? std::make_shared<ClassVal>(def->name, env, def->body, def->extends) : std::make_shared<ClassVal>(def->name, env, def->body));
}