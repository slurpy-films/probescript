#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

Val evalObject(ObjectLiteralType* obj, Env* env) {
    std::shared_ptr<ObjectVal> object = std::make_shared<ObjectVal>();
    for (PropertyLiteralType* property : obj->properties) {
        Val runtimeval = (property->val == nullptr) ? env->lookupVar(property->key) : eval(property->val, env);
        object->properties[property->key] = runtimeval;
    }

    return object;
}