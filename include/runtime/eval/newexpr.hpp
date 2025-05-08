#pragma once
#include <string>
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"
#include "runtime/interpreter.hpp"
#include "call.hpp"
#include "fndeclaration.hpp"
#include "assignment.hpp"

void inheritClass(std::shared_ptr<ClassVal> cls, Env* env, std::shared_ptr<ObjectVal> thisObj, std::vector<Val> args);

Val evalNewExpr(NewExprType* newexpr, Env* env) {
    Val rawcls = eval(newexpr->constructor, env);

    if (rawcls->type == ValueType::NativeClass) {
        std::shared_ptr<NativeClassVal> natcls = std::static_pointer_cast<NativeClassVal>(rawcls);
        std::vector<Val> args;

        for (Expr* expr : newexpr->args) {
            args.push_back(eval(expr, env));
        }

        return natcls->constructor(args, new Env(env));
    }

    if (rawcls->type != ValueType::Class) {
        std::cerr << "Cannot construct non class value";
        exit(1);
    }

    std::shared_ptr<ClassVal> cls = std::static_pointer_cast<ClassVal>(rawcls);
    std::vector<Val> args;

    for (Expr* expr : newexpr->args) {
        args.push_back(eval(expr, env));
    }
    
    Env* scope = new Env(env);
    std::shared_ptr<ObjectVal> thisObj = std::make_shared<ObjectVal>();
    scope->declareVar("this", thisObj);

    inheritClass(cls, scope, thisObj, args);

    for (Stmt* stmt : cls->body) {
        switch (stmt->kind) {
            case NodeType::FunctionDeclaration: {
                std::shared_ptr<FunctionValue> fnval = std::static_pointer_cast<FunctionValue>(evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(stmt), scope, true));
                std::shared_ptr<ObjectVal> thisobj = std::static_pointer_cast<ObjectVal>(scope->lookupVar("this"));
                thisobj->properties[fnval->name] = fnval;
                scope->assignVar("this", thisobj);
                break;
            }
            case NodeType::VarDeclaration: {
                VarDeclarationType* var = static_cast<VarDeclarationType*>(stmt);
                std::shared_ptr<ObjectVal> thisobj = std::static_pointer_cast<ObjectVal>(scope->lookupVar("this"));
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

                assignEnv->declareVar(static_cast<IdentifierType*>(assign->assigne)->symbol, std::make_shared<UndefinedVal>());

                evalAssignment(assign, assignEnv);

                std::shared_ptr<ObjectVal> thisObj = std::static_pointer_cast<ObjectVal>(scope->lookupVar("this"));

                thisObj->properties[static_cast<IdentifierType*>(assign->assigne)->symbol] = assignEnv->variables[static_cast<IdentifierType*>(assign->assigne)->symbol];
                scope->assignVar("this", thisObj);
                break;
            }
        
            default:
                std::cerr << "Only variable and function declarations are allowed within class bodies";
                exit(1);
        }
    }
    

    if (std::static_pointer_cast<ObjectVal>(scope->variables["this"])->properties.find("constructor") != std::static_pointer_cast<ObjectVal>(scope->variables["this"])->properties.end()) {
        Val constructor = std::static_pointer_cast<ObjectVal>(scope->lookupVar("this"))->properties["constructor"];
        evalCallWithFnVal(constructor, args, scope);
    }

    return scope->lookupVar("this");
}

void inheritClass(std::shared_ptr<ClassVal> cls, Env* env, std::shared_ptr<ObjectVal> thisObj, std::vector<Val> args) {
    if (!cls->doesExtend) return;

    Val extendsVal = eval(cls->extends, cls->parentEnv);
    if (extendsVal->type != ValueType::Class) {
        std::cerr << "Superclass must be a class.\n";
        exit(1);
    }
    Env* superScope = new Env(cls->parentEnv);
    std::shared_ptr<ClassVal> superClass = std::static_pointer_cast<ClassVal>(extendsVal);
    superScope->declareVar("this", thisObj);

    inheritClass(superClass, superScope, thisObj, args);

    Val cons;
    bool hasCons = false;

    for (Stmt* stmt : superClass->body) {
        if (stmt->kind == NodeType::FunctionDeclaration) {
            std::shared_ptr<FunctionValue> fnval = std::static_pointer_cast<FunctionValue>(evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(stmt), superScope, true));
            if (fnval->name == "constructor") {
                cons = fnval;
                hasCons = true;
            } else thisObj->properties[fnval->name] = fnval;
        } else if (stmt->kind == NodeType::AssignmentExpr) {
            AssignmentExprType* assign = static_cast<AssignmentExprType*>(stmt);
            if (assign->op != "=") {
                std::cerr << "Only = assignment is allowed in class bodies";
                exit(1);
            }

            Env* assignEnv = new Env();
            assignEnv->declareVar(static_cast<IdentifierType*>(assign->assigne)->symbol, std::make_shared<UndefinedVal>());
            evalAssignment(assign, assignEnv);
            thisObj->properties[static_cast<IdentifierType*>(assign->assigne)->symbol] = assignEnv->variables[static_cast<IdentifierType*>(assign->assigne)->symbol];
        }
    }

    if (hasCons) {
        env->declareVar("super", cons);
    }

}