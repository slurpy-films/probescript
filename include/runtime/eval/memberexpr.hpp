#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

RuntimeVal* evalMemberExpr(MemberExprType* expr, Env* env) {
    RuntimeVal* obj = eval(expr->object, env);

    if (obj->type != ValueType::Object) {
        cerr << "Cannot find member of non object";
        exit(1);
    }

    string key;

    if (expr->computed) {
        RuntimeVal* propValue = eval(expr->property, env);

        if (propValue->type != ValueType::String) {
            cerr << "Computed property must evaluate to a string";
            exit(1);
        }

        key = static_cast<StringVal*>(propValue)->string;
    } else {
        IdentifierType* ident = static_cast<IdentifierType*>(expr->property);
        key = ident->symbol;
    }

    ObjectVal* objectVal = static_cast<ObjectVal*>(obj);

    string objName = static_cast<IdentifierType*>(expr->object)->symbol;

    ObjectVal* object = static_cast<ObjectVal*>(env->lookupVar(objName));

    

    if (object->properties.count(key) == 0) {
        cerr << "Object has no property " << key;
        exit(1);
    }

    return object->properties[key];
}