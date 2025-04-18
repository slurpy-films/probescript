#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

RuntimeVal* evalObject(ObjectLiteralType* obj, Env* env) {
    ObjectVal* object = new ObjectVal();
    for (PropertyLiteralType* property : obj->properties) {
        RuntimeVal* runtimeval = (property->val == nullptr) ? env->lookupVar(property->key) : eval(property->val, env);
        object->properties[property->key] = runtimeval;
    }

    return object;
}