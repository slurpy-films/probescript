#include "runtime/interpreter.hpp"

Val evalClassDefinition(std::shared_ptr<ClassDefinitionType> def, EnvPtr env) {
    return env->declareVar(def->name, def->doesExtend ? makeVal<ClassVal>(def->token, def->name, env, def->body, def->extends) : makeVal<ClassVal>(def->token, def->name, env, def->body), def->token);
}

Val evalNewExpr(std::shared_ptr<NewExprType> newexpr, EnvPtr env) {
    Val rawcls = eval(newexpr->constructor, env);

    if (rawcls->type == ValueType::NativeClass) {
        std::shared_ptr<NativeClassVal> natcls = std::static_pointer_cast<NativeClassVal>(rawcls);
        std::vector<Val> args;

        for (std::shared_ptr<Expr> expr : newexpr->args) {
            args.push_back(eval(expr, env));
        }

        return natcls->constructor(args, std::make_shared<Env>(env));
    }

    if (rawcls->type != ValueType::Class) {
        throw ThrowException("Cannot construct non class value");
    }

    std::shared_ptr<ClassVal> cls = std::static_pointer_cast<ClassVal>(rawcls);
    std::vector<Val> args;

    for (std::shared_ptr<Expr> expr : newexpr->args) {
        args.push_back(eval(expr, env));
    }
    
    EnvPtr scope = std::make_shared<Env>(env);
    std::shared_ptr<ObjectVal> thisObj = std::make_shared<ObjectVal>();
    scope->declareVar("this", thisObj, cls->token);

    inheritClass(cls, scope, thisObj, args);

    for (std::shared_ptr<Stmt> stmt : cls->body) {
        switch (stmt->kind) {
            case NodeType::FunctionDeclaration: {
                std::shared_ptr<FunctionValue> fnval = std::static_pointer_cast<FunctionValue>(evalFunctionDeclaration(std::static_pointer_cast<FunctionDeclarationType>(stmt), scope, true));
                thisObj->properties[fnval->name] = fnval;
                break;
            }
            case NodeType::VarDeclaration: {
                std::shared_ptr<VarDeclarationType> var = std::static_pointer_cast<VarDeclarationType>(stmt);
                thisObj->properties[var->identifier] = eval(var->value, scope);
                break;
            }
            case NodeType::AssignmentExpr: {
                std::shared_ptr<AssignmentExprType> assign = std::static_pointer_cast<AssignmentExprType>(stmt);
                if (assign->op != "=") {
                    throw ThrowException(CustomError("Only = assignment is allowed in class bodies", "ClassBodyError"));
                }

                EnvPtr assignEnv = std::make_shared<Env>();

                assignEnv->declareVar(std::static_pointer_cast<IdentifierType>(assign->assigne)->symbol, std::make_shared<UndefinedVal>(), assign->token);

                evalAssignment(assign, assignEnv);

                thisObj->properties[std::static_pointer_cast<IdentifierType>(assign->assigne)->symbol] = assignEnv->variables[std::static_pointer_cast<IdentifierType>(assign->assigne)->symbol];
                break;
            }
        
            default:
                eval(stmt, env);
        }
    }
    

    if (std::static_pointer_cast<ObjectVal>(scope->variables["this"])->properties.find("new") != std::static_pointer_cast<ObjectVal>(scope->variables["this"])->properties.end())
    {
        Val constructor = thisObj->properties["new"];
        evalCallWithFnVal(constructor, args, scope);
    }

    return scope->lookupVar("this", newexpr->token);
}

void inheritClass(std::shared_ptr<ClassVal> cls, EnvPtr env, std::shared_ptr<ObjectVal> thisObj, std::vector<Val> args)
{
    if (!cls->doesExtend) return;

    Val extendsVal = eval(cls->extends, cls->parentEnv);
    if (extendsVal->type != ValueType::Class)
    {
        throw ThrowException(CustomError("Superclass must be a class", "ClassInheritanceError"));
        return;
    }

    EnvPtr superScope = std::make_shared<Env>(cls->parentEnv);
    std::shared_ptr<ClassVal> superClass = std::static_pointer_cast<ClassVal>(extendsVal);
    superScope->declareVar("this", thisObj, superClass->token);

    inheritClass(superClass, superScope, thisObj, args);

    Val cons;
    bool hasCons = false;

    for (std::shared_ptr<Stmt> stmt : superClass->body)
    {
        if (stmt->kind == NodeType::FunctionDeclaration)
        {
            std::shared_ptr<FunctionValue> fnval = std::static_pointer_cast<FunctionValue>(evalFunctionDeclaration(std::static_pointer_cast<FunctionDeclarationType>(stmt), superScope, true));
            if (fnval->name == "new")
            {
                cons = fnval;
                hasCons = true;
            } else thisObj->properties[fnval->name] = fnval;
        } else if (stmt->kind == NodeType::VarDeclaration)
        {
            std::shared_ptr<VarDeclarationType> decl = std::static_pointer_cast<VarDeclarationType>(stmt);

            thisObj->properties[decl->identifier] = eval(decl->value, superScope);
        } else
        {
            eval(stmt, superScope);
        }
    }


    if (hasCons)
    {
        env->declareVar("super", cons, cls->extends->token);
    }
}