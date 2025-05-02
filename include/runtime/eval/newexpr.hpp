#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "call.hpp"
#include "fndeclaration.hpp"

RuntimeVal* evalNewExpr(NewExprType* newexpr, Env* env) {
    RuntimeVal* rawcls = eval(newexpr->constructor, env);

    if (rawcls->type != ValueType::Class) {
        std::cerr << "Cannot construct non class value";
        exit(1);
    }

    ClassVal* cls = static_cast<ClassVal*>(rawcls);
    std::vector<RuntimeVal*> args;

    for (Expr* expr : newexpr->args) {
        args.push_back(eval(expr, env));
    }
    
    Env* scope = new Env(env);
    scope->declareVar("this", new ObjectVal());

    for (Stmt* stmt : cls->body) {
        switch (stmt->kind) {
            case NodeType::FunctionDeclaration: {
                FunctionValue* fnval = static_cast<FunctionValue*>(evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(stmt), scope, true));
                ObjectVal* thisobj = static_cast<ObjectVal*>(scope->lookupVar("this"));
                thisobj->properties[fnval->name] = fnval;
                scope->assignVar("this", thisobj);
                break;
            }
            case NodeType::VarDeclaration: {
                VarDecalarationType* var = static_cast<VarDecalarationType*>(stmt);
                ObjectVal* thisobj = static_cast<ObjectVal*>(scope->lookupVar("this"));
                thisobj->properties[var->identifier] = eval(var->value, scope);
                scope->assignVar("this", thisobj);
                break;
            }
        
            default:
                std::cerr << "Only var and function declarations are allowed within function bodies";
                exit(1);
        }
    }

    if (static_cast<ObjectVal*>(scope->variables["this"])->properties.find("constructor") != static_cast<ObjectVal*>(scope->variables["this"])->properties.end()) {
        RuntimeVal* constructor = static_cast<ObjectVal*>(scope->lookupVar("this"))->properties["constructor"];
        evalCallWithFnVal(constructor, args, scope);
    }

    return scope->lookupVar("this");
}