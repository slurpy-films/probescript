#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

RuntimeVal* evalMemberAssignment(MemberAssignmentType* expr, Env* env) {
    RuntimeVal* obj = eval(expr->object, env);
    RuntimeVal* value = eval(expr->newvalue, env);

    if (obj->type != ValueType::Object) {
        std::cerr << "Cannot find member of non object";
        exit(1);
    }

    std::string key;

    if (expr->computed) {
        RuntimeVal* propValue = eval(expr->property, env);

        if (propValue->type != ValueType::String) {
            std::cerr << "Computed property must evaluate to a string";
            exit(1);
        }

        key = static_cast<StringVal*>(propValue)->string;
    } else {
        IdentifierType* ident = static_cast<IdentifierType*>(expr->property);
        key = ident->symbol;
    }

    ObjectVal* objectVal = static_cast<ObjectVal*>(obj);

    std::string objName = static_cast<IdentifierType*>(expr->object)->symbol;

    ObjectVal* object = static_cast<ObjectVal*>(env->lookupVar(objName));

    
    object->properties[key] = value;
    
    return env->assignVar(objName, object);
}