#include "runtime/interpreter.hpp"

using namespace Probescript;
using namespace Probescript::Interpreter;

Values::Val Interpreter::evalClassDefinition(std::shared_ptr<AST::ClassDefinitionType> def, EnvPtr env)
{
    return env->declareVar(def->name, def->doesExtend ? Values::makeVal<Values::ClassVal>(def->token, def->name, env, def->body, def->extends) : Values::makeVal<Values::ClassVal>(def->token, def->name, env, def->body), def->token);
}

Values::Val Interpreter::evalNewExpr(std::shared_ptr<AST::NewExprType> newexpr, EnvPtr env)
{
    Values::Val rawcls = eval(newexpr->constructor, env);

    if (rawcls->type == Values::ValueType::NativeClass)
    {
        std::shared_ptr<Values::NativeClassVal> natcls = std::static_pointer_cast<Values::NativeClassVal>(rawcls);
        std::vector<Values::Val> args;

        for (std::shared_ptr<AST::Expr> expr : newexpr->args)
        {
            args.push_back(eval(expr, env));
        }

        return natcls->constructor(args, std::make_shared<Env>(env));
    }

    if (rawcls->type != Values::ValueType::Class)
    {
        throw ThrowException(CustomError("Cannot construct non class value", "NewError", newexpr->constructor->token));
    }

    std::shared_ptr<Values::ClassVal> cls = std::static_pointer_cast<Values::ClassVal>(rawcls);
    std::vector<Values::Val> args;

    for (std::shared_ptr<AST::Expr> expr : newexpr->args)
    {
        args.push_back(eval(expr, env));
    }
    
    EnvPtr scope = std::make_shared<Env>(env);
    std::shared_ptr<Values::ObjectVal> thisObj = std::make_shared<Values::ObjectVal>();
    scope->declareVar("this", thisObj, cls->token);

    inheritClass(cls, scope, thisObj, args);

    for (std::shared_ptr<AST::Stmt> stmt : cls->body) {
        switch (stmt->kind) {
            case AST::NodeType::FunctionDeclaration: {
                std::shared_ptr<Values::FunctionValue> fnval = std::static_pointer_cast<Values::FunctionValue>(evalFunctionDeclaration(std::static_pointer_cast<AST::FunctionDeclarationType>(stmt), scope, true));
                thisObj->properties[fnval->name] = fnval;
                break;
            }
            case AST::NodeType::VarDeclaration: {
                std::shared_ptr<AST::VarDeclarationType> var = std::static_pointer_cast<AST::VarDeclarationType>(stmt);
                thisObj->properties[var->identifier] = eval(var->value, scope);
                break;
            }
            case AST::NodeType::AssignmentExpr: {
                std::shared_ptr<AST::AssignmentExprType> assign = std::static_pointer_cast<AST::AssignmentExprType>(stmt);
                if (assign->op != "=") {
                    throw ThrowException(CustomError("Only = assignment is allowed in class bodies", "ClassBodyError"));
                }

                EnvPtr assignEnv = std::make_shared<Env>();

                assignEnv->declareVar(std::static_pointer_cast<AST::IdentifierType>(assign->assigne)->symbol, std::make_shared<Values::UndefinedVal>(), assign->token);

                evalAssignment(assign, assignEnv);

                thisObj->properties[std::static_pointer_cast<AST::IdentifierType>(assign->assigne)->symbol] = assignEnv->variables[std::static_pointer_cast<AST::IdentifierType>(assign->assigne)->symbol];
                break;
            }
        
            default:
                eval(stmt, env);
        }
    }
    

    if (std::static_pointer_cast<Values::ObjectVal>(scope->variables["this"])->properties.find("new") != std::static_pointer_cast<Values::ObjectVal>(scope->variables["this"])->properties.end())
    {
        Values::Val constructor = thisObj->properties["new"];
        evalCallWithFnVal(constructor, args, scope);
    }

    return scope->lookupVar("this", newexpr->token);
}

void Interpreter::inheritClass(std::shared_ptr<Values::ClassVal> cls, EnvPtr env, std::shared_ptr<Values::ObjectVal> thisObj, std::vector<Values::Val> args)
{
    if (!cls->doesExtend) return;

    Values::Val extendsVal = eval(cls->extends, cls->parentEnv);
    if (extendsVal->type != Values::ValueType::Class)
    {
        throw ThrowException(CustomError("Superclass must be a class", "ClassInheritanceError"));
        return;
    }

    EnvPtr superScope = std::make_shared<Env>(cls->parentEnv);
    std::shared_ptr<Values::ClassVal> superClass = std::static_pointer_cast<Values::ClassVal>(extendsVal);
    superScope->declareVar("this", thisObj, superClass->token);

    inheritClass(superClass, superScope, thisObj, args);

    Values::Val cons;

    for (std::shared_ptr<AST::Stmt> stmt : superClass->body)
    {
        if (stmt->kind == AST::NodeType::FunctionDeclaration)
        {
            std::shared_ptr<Values::FunctionValue> fnval = std::static_pointer_cast<Values::FunctionValue>(evalFunctionDeclaration(std::static_pointer_cast<AST::FunctionDeclarationType>(stmt), superScope, true));
            if (fnval->name == "new")
            {
                cons = fnval;
            } else thisObj->properties[fnval->name] = fnval;
        } else if (stmt->kind == AST::NodeType::VarDeclaration)
        {
            std::shared_ptr<AST::VarDeclarationType> decl = std::static_pointer_cast<AST::VarDeclarationType>(stmt);

            thisObj->properties[decl->identifier] = eval(decl->value, superScope);
        } else
        {
            eval(stmt, superScope);
        }
    }


    if (cons != nullptr)
    {
        env->declareVar("super", cons, cls->extends->token);
    }
}