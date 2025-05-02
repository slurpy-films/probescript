#pragma once
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "runtime/interpreter.hpp"

RuntimeVal* evalClassDefinition(ClassDefinitionType* def, Env* env) {
    std::vector<RuntimeVal*> body;

    return env->declareVar(def->name, def->doesExtend ? new ClassVal(def->name, env, def->body, def->extends) : new ClassVal(def->name, env, def->body));
}