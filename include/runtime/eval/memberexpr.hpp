#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "stdlib/array.hpp"

RuntimeVal* evalMemberExpr(MemberExprType* expr, Env* env) {
    RuntimeVal* obj = eval(expr->object, env);

    if (obj->type == ValueType::Object) {
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

        string objName = static_cast<IdentifierType*>(expr->object)->symbol;

        ObjectVal* object = static_cast<ObjectVal*>(env->lookupVar(objName));

        if (object->properties.count(key) == 0) {
            cerr << "Object has no property " << key;
            exit(1);
        }

        return object->properties[key];
    } else if (obj->type == ValueType::Array) {
        if (!expr->computed) {
            IdentifierType* ident = static_cast<IdentifierType*>(expr->property);
            if (ident->symbol == "size") {
                return arraySize(static_cast<IdentifierType*>(expr->object)->symbol);
            }
        }

        RuntimeVal* indexval = eval(expr->property, env);

        if (indexval->type != ValueType::Number) {
            cerr << "Array index must evaluate to a number";
            exit(1);
        }

        NumberVal* index = static_cast<NumberVal*>(indexval);

        string arrayName = static_cast<IdentifierType*>(expr->object)->symbol;

        ArrayVal* array = static_cast<ArrayVal*>(env->lookupVar(arrayName));

        int idx = stoi(index->number);

        if (idx < 0 || idx >= static_cast<int>(array->items.size())) {
            return new UndefinedVal();
        }

        return array->items[idx];
    } else {
        cerr << "Cannot evaluate member expression of type " << obj->type;
        exit(1);
    }
}