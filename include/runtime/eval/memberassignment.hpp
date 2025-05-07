#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"

Val evalMemberAssignment(MemberAssignmentType* expr, Env* env) {
    Val obj = eval(expr->object, env);
    Val value = eval(expr->newvalue, env);

    std::string key;

    if (expr->computed) {
        Val propValue = eval(expr->property, env);

        if (propValue->type == ValueType::Number) {
            int index = std::static_pointer_cast<NumberVal>(propValue)->toNum();

            if (obj->type == ValueType::Array) {
                std::shared_ptr<ArrayVal> array = std::static_pointer_cast<ArrayVal>(obj);

                if (index >= array->items.size()) {
                    array->items.resize(index + 1, std::make_shared<UndefinedVal>());
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

        key = std::static_pointer_cast<StringVal>(propValue)->string;
    } else {
        IdentifierType* ident = static_cast<IdentifierType*>(expr->property);
        key = ident->symbol;
    }

    if (obj->type == ValueType::Object) {
        std::shared_ptr<ObjectVal> objectVal = std::static_pointer_cast<ObjectVal>(obj);
        objectVal->properties[key] = value;
        return objectVal;
    }

    std::cerr << "Cannot assign member to non-object/non-array value";
    exit(1);
}
