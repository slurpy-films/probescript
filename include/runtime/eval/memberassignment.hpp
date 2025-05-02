#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

RuntimeVal* evalMemberAssignment(MemberAssignmentType* expr, Env* env) {
    RuntimeVal* obj = eval(expr->object, env);
    RuntimeVal* value = eval(expr->newvalue, env);

    std::string key;

    if (expr->computed) {
        RuntimeVal* propValue = eval(expr->property, env);

        if (propValue->type == ValueType::Number) {
            int index = std::stoi(static_cast<NumberVal*>(propValue)->number);

            if (obj->type == ValueType::Array) {
                ArrayVal* array = static_cast<ArrayVal*>(obj);

                if (index >= array->items.size()) {
                    array->items.resize(index + 1, new UndefinedVal());
                }

                array->items[index] = value;
                return array;
            } else {
                std::cerr << "Cannot use numeric index on non-array object";
                exit(1);
            }
        }

        if (propValue->type != ValueType::String) {
            std::cerr << "Computed property must evaluate to a string or number";
            exit(1);
        }

        key = static_cast<StringVal*>(propValue)->string;
    } else {
        IdentifierType* ident = static_cast<IdentifierType*>(expr->property);
        key = ident->symbol;
    }

    if (obj->type == ValueType::Object) {
        ObjectVal* objectVal = static_cast<ObjectVal*>(obj);
        objectVal->properties[key] = value;
        return objectVal;
    }

    std::cerr << "Cannot assign member to non-object/non-array value";
    exit(1);
}
