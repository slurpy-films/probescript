#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "stdlib/array.hpp"

Val evalMemberExpr(MemberExprType* expr, Env* env) {
    Val obj = eval(expr->object, env);
    
    if (obj->type == ValueType::Object) {
        std::string key;

        if (expr->computed) {
            Val propValue = eval(expr->property, env);
    
            if (propValue->type != ValueType::String) {
                std::cerr << "Computed property must evaluate to a string";
                exit(1);
            }
    
            key = std::static_pointer_cast<StringVal>(propValue)->string;
        } else {
            IdentifierType* ident = static_cast<IdentifierType*>(expr->property);
            key = ident->symbol;
        }

        std::shared_ptr<ObjectVal> object = std::static_pointer_cast<ObjectVal>(obj);

        if (object->properties.count(key) == 0) {
            std::cerr << "Object has no property " << key;
            exit(1);
        }

        return object->properties[key];
    } else if (obj->type == ValueType::Array) {
        if (!expr->computed) {
            IdentifierType* ident = static_cast<IdentifierType*>(expr->property);
            if (ident->symbol == "size") {
                return arraySize(static_cast<IdentifierType*>(expr->object)->symbol);
            } else if (ident->symbol == "push") {
                return arrayPush(static_cast<IdentifierType*>(expr->object)->symbol);
            }
        }

        Val indexval = eval(expr->property, env);

        if (indexval->type != ValueType::Number) {
            std::cerr << "Array index must evaluate to a number";
            exit(1);
        }

        std::shared_ptr<NumberVal> index = std::static_pointer_cast<NumberVal>(indexval);

        std::shared_ptr<ArrayVal> array = std::static_pointer_cast<ArrayVal>(obj);

        int idx = index->number;

        if (idx < 0 || idx >= static_cast<int>(array->items.size())) {
            return std::make_shared<UndefinedVal>();
        }

        return array->items[idx];
    } else {
        std::cerr << "Cannot evaluate member expression of type " << obj->type;
        exit(1);
    }
}