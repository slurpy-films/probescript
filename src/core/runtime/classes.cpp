#include "runtime/interpreter.hpp"

Val evalClassDefinition(ClassDefinitionType* def, EnvPtr env) {
    return env->declareVar(def->name, def->doesExtend ? std::make_shared<ClassVal>(def->name, env, def->body, def->extends) : std::make_shared<ClassVal>(def->name, env, def->body));
}

Val evalNewExpr(NewExprType* newexpr, EnvPtr env) {
    Val rawcls = eval(newexpr->constructor, env);

    if (rawcls->type == ValueType::NativeClass) {
        std::shared_ptr<NativeClassVal> natcls = std::static_pointer_cast<NativeClassVal>(rawcls);
        std::vector<Val> args;

        for (Expr* expr : newexpr->args) {
            args.push_back(eval(expr, env));
        }

        return natcls->constructor(args, std::make_shared<Env>(env));
    }

    if (rawcls->type != ValueType::Class) {
        return env->throwErr("Cannot construct non class value");
    }

    std::shared_ptr<ClassVal> cls = std::static_pointer_cast<ClassVal>(rawcls);
    std::vector<Val> args;

    for (Expr* expr : newexpr->args) {
        args.push_back(eval(expr, env));
    }
    
    EnvPtr scope = std::make_shared<Env>(env);
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
                    return env->throwErr(ManualError("Only = assignment is allowed in class bodies", "ClassBodyError"));
                }

                EnvPtr assignEnv = std::make_shared<Env>();

                assignEnv->declareVar(static_cast<IdentifierType*>(assign->assigne)->symbol, std::make_shared<UndefinedVal>());

                evalAssignment(assign, assignEnv);

                std::shared_ptr<ObjectVal> thisObj = std::static_pointer_cast<ObjectVal>(scope->lookupVar("this"));

                thisObj->properties[static_cast<IdentifierType*>(assign->assigne)->symbol] = assignEnv->variables[static_cast<IdentifierType*>(assign->assigne)->symbol];
                scope->assignVar("this", thisObj);
                break;
            }
        
            default:
                eval(stmt, env);
        }
    }
    

    if (std::static_pointer_cast<ObjectVal>(scope->variables["this"])->properties.find("new") != std::static_pointer_cast<ObjectVal>(scope->variables["this"])->properties.end())
    {
        Val constructor = std::static_pointer_cast<ObjectVal>(scope->lookupVar("this"))->properties["new"];
        evalCallWithFnVal(constructor, args, scope);
    }

    return scope->lookupVar("this");
}

void inheritClass(std::shared_ptr<ClassVal> cls, EnvPtr env, std::shared_ptr<ObjectVal> thisObj, std::vector<Val> args)
{
    if (!cls->doesExtend) return;

    Val extendsVal = eval(cls->extends, cls->parentEnv);
    if (extendsVal->type != ValueType::Class)
    {
        env->throwErr(ManualError("Superclass must be a class", "ClassInheritanceError"));
        return;
    }

    EnvPtr superScope = std::make_shared<Env>(cls->parentEnv);
    std::shared_ptr<ClassVal> superClass = std::static_pointer_cast<ClassVal>(extendsVal);
    superScope->declareVar("this", thisObj);

    inheritClass(superClass, superScope, thisObj, args);

    Val cons;
    bool hasCons = false;

    for (Stmt* stmt : superClass->body)
    {
        if (stmt->kind == NodeType::FunctionDeclaration)
        {
            std::shared_ptr<FunctionValue> fnval = std::static_pointer_cast<FunctionValue>(evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(stmt), superScope, true));
            if (fnval->name == "new")
            {
                cons = fnval;
                hasCons = true;
            } else thisObj->properties[fnval->name] = fnval;
        } else if (stmt->kind == NodeType::VarDeclaration)
        {
            VarDeclarationType* decl = static_cast<VarDeclarationType*>(stmt);

            thisObj->properties[decl->identifier] = eval(decl->value, superScope);
        } else
        {
            eval(stmt, superScope);
        }
    }


    if (hasCons)
    {
        env->declareVar("super", cons);
    }
}