#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "call.hpp"
#include "fndeclaration.hpp"
#include "assignment.hpp"

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


    
    if (cls->doesExtend) {
        RuntimeVal* extendsVal = eval(cls->extends, cls->parentEnv);
        if (extendsVal->type != ValueType::Class) {
            std::cerr << "Superclass must be a class.\n";
            exit(1);
        }
    
        ClassVal* superClass = static_cast<ClassVal*>(extendsVal);
    
        ObjectVal* childThis = static_cast<ObjectVal*>(scope->lookupVar("this"));
        Env* tempScope = new Env(cls->parentEnv);
        tempScope->declareVar("this", childThis);
        
    
        for (Stmt* stmt : superClass->body) {
            if (stmt->kind == NodeType::FunctionDeclaration) {
                FunctionValue* fnval = static_cast<FunctionValue*>(evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(stmt), tempScope, true));
                childThis->properties[fnval->name] = fnval;
            } else if (stmt->kind == NodeType::AssignmentExpr) {
                AssignmentExprType* assign = static_cast<AssignmentExprType*>(stmt);
                if (assign->op != "=") {
                    std::cerr << "Only = assignment is allowed in class bodies";
                    exit(1);
                }

                Env* assignEnv = new Env();

                assignEnv->declareVar(static_cast<IdentifierType*>(assign->assigne)->symbol, new UndefinedVal());

                evalAssignment(assign, assignEnv);

                childThis->properties[static_cast<IdentifierType*>(assign->assigne)->symbol] = assignEnv->variables[static_cast<IdentifierType*>(assign->assigne)->symbol];
            }
        }
    
        if (childThis->properties.find("constructor") != childThis->properties.end()) {
            scope->declareVar("super", childThis->properties["constructor"]);
        }

        ObjectVal* subThis = static_cast<ObjectVal*>(scope->lookupVar("this"));

        for (const auto& [key, val] : static_cast<ObjectVal*>(tempScope->lookupVar("this"))->properties) {
            if (subThis->properties.find(key) == subThis->properties.end()) {
                subThis->properties[key] = val;
            }
        }

        scope->assignVar("this", subThis);
    }

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
            case NodeType::AssignmentExpr: {
                AssignmentExprType* assign = static_cast<AssignmentExprType*>(stmt);
                if (assign->op != "=") {
                    std::cerr << "Only = assignment is allowed in class bodies";
                    exit(1);
                }

                Env* assignEnv = new Env();

                assignEnv->declareVar(static_cast<IdentifierType*>(assign->assigne)->symbol, new UndefinedVal());

                evalAssignment(assign, assignEnv);

                ObjectVal* thisObj = static_cast<ObjectVal*>(scope->lookupVar("this"));

                thisObj->properties[static_cast<IdentifierType*>(assign->assigne)->symbol] = assignEnv->variables[static_cast<IdentifierType*>(assign->assigne)->symbol];
                scope->assignVar("this", thisObj);
                break;
            }
        
            default:
                std::cerr << "Only variable and function declarations are allowed within class bodies";
                exit(1);
        }
    }
    

    if (static_cast<ObjectVal*>(scope->variables["this"])->properties.find("constructor") != static_cast<ObjectVal*>(scope->variables["this"])->properties.end()) {
        RuntimeVal* constructor = static_cast<ObjectVal*>(scope->lookupVar("this"))->properties["constructor"];
        evalCallWithFnVal(constructor, args, scope);
    }

    return scope->lookupVar("this");
}