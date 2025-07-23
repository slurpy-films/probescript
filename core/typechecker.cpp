#include "typechecker.hpp"

using namespace Probescript;
using namespace Probescript::Typechecker;

// Define global types
TypePtr Typechecker::g_anyty = std::make_shared<Type>(TypeKind::Any, "any");
TypePtr Typechecker::g_numty = std::make_shared<Type>(TypeKind::Number, "number");
TypePtr Typechecker::g_strty = std::make_shared<Type>(TypeKind::String, "string");
TypePtr Typechecker::g_boolty = std::make_shared<Type>(TypeKind::Bool, "bool");
TypePtr Typechecker::g_arrayty = std::make_shared<Type>(TypeKind::String, "string");
TypePtr Typechecker::g_mapty = std::make_shared<Type>(TypeKind::Bool, "bool");

TypeEnv::TypeEnv(std::shared_ptr<TypeEnv> parent)
{
    m_parent = parent;

    for (const auto& [key, type] : g_typeGlobals)
        m_variables[key] = type;
}

std::unordered_map<std::string, TypePtr> TypeEnv::getVars()
{
    return m_variables;
}

TypePtr TypeEnv::declareVar(std::string name, TypePtr type, Lexer::Token tk)
{
    if (m_variables.find(name) != m_variables.end())
    {
        throw std::runtime_error(CustomError("Variable " + name + " is already defined", "RedefinitionError", tk));
    }

    m_variables[name] = type;

    return type;
}

TypePtr TypeEnv::lookUp(std::string name, Lexer::Token tk)
{
    if (m_variables.find(name) != m_variables.end())
        return m_variables[name];
    else if (m_parent)
        return m_parent->lookUp(name, tk);
    else
    {
        throw std::runtime_error(CustomError("Variable " + name + " is not defined", "ReferenceError", tk));
    }
}

void TypeEnv::massDeclare(std::unordered_map<std::string, TypePtr> vars)
{
    for (const auto& pair : vars)
    {
        m_variables.insert(pair);
    }
}

void TC::checkProgram(std::shared_ptr<AST::ProgramType> program, TypeEnvPtr env, std::shared_ptr<Context> ctx)
{
    m_context = ctx;
    for (std::shared_ptr<AST::Stmt> stmt : program->body)
    {
        check(stmt, env, ctx);
    }
}

TypePtr TC::check(std::shared_ptr<AST::Stmt> node, TypeEnvPtr env, std::shared_ptr<Context> ctx)
{
    if (!node) return std::make_shared<Type>(TypeKind::Any, "any");
    switch (node->kind)
    {
        case AST::NodeType::NumericLiteral:
            return std::make_shared<Type>(TypeKind::Number, "number");
        case AST::NodeType::StringLiteral:
            return std::make_shared<Type>(TypeKind::String, "string");
        case AST::NodeType::BoolLiteral:
            return std::make_shared<Type>(TypeKind::Bool, "bool");
        case AST::NodeType::VarDeclaration:
            return checkVarDecl(std::static_pointer_cast<AST::VarDeclarationType>(node), env);
        case AST::NodeType::AssignmentExpr:
            return checkAssign(std::static_pointer_cast<AST::AssignmentExprType>(node), env);
        case AST::NodeType::Identifier:
            return checkIdent(std::static_pointer_cast<AST::IdentifierType>(node), env);
        case AST::NodeType::FunctionDeclaration:
            return checkFunction(std::static_pointer_cast<AST::FunctionDeclarationType>(node), env);
        case AST::NodeType::ProbeDeclaration:
            return checkProbe(std::static_pointer_cast<AST::ProbeDeclarationType>(node), env);
        case AST::NodeType::CallExpr:
            return checkCall(std::static_pointer_cast<AST::CallExprType>(node), env);
        case AST::NodeType::MapLiteral:
            return checkObjectExpr(std::static_pointer_cast<AST::MapLiteralType>(node), env);
        case AST::NodeType::MemberExpr:
            return checkMemberExpr(std::static_pointer_cast<AST::MemberExprType>(node), env);
        case AST::NodeType::ImportStmt:
            return checkImportStmt(std::static_pointer_cast<AST::ImportStmtType>(node), env, ctx);
        case AST::NodeType::ExportStmt:
            return checkExportStmt(std::static_pointer_cast<AST::ExportStmtType>(node)->exporting, env, ctx);
        case AST::NodeType::BinaryExpr:
            return checkBinExpr(std::static_pointer_cast<AST::BinaryExprType>(node), env);
        case AST::NodeType::ClassDefinition:
            return checkClassDeclaration(std::static_pointer_cast<AST::ClassDefinitionType>(node), env);
        case AST::NodeType::NewExpr:
            return checkNewExpr(std::static_pointer_cast<AST::NewExprType>(node), env);
        case AST::NodeType::IfStmt:
            return checkIfStmt(std::static_pointer_cast<AST::IfStmtType>(node), env);
        case AST::NodeType::ForStmt:
            return checkForStmt(std::static_pointer_cast<AST::ForStmtType>(node), env);
        case AST::NodeType::MemberAssignment:
            return checkMemberAssign(std::static_pointer_cast<AST::MemberAssignmentType>(node), env);
        case AST::NodeType::ArrayLiteral:
            return checkArrayLiteral(std::static_pointer_cast<AST::ArrayLiteralType>(node), env);
        case AST::NodeType::ArrowFunction:
            return checkArrowFunction(std::static_pointer_cast<AST::ArrowFunctionType>(node), env);
        case AST::NodeType::TernaryExpr:
            return checkTernaryExpr(std::static_pointer_cast<AST::TernaryExprType>(node), env);
        case AST::NodeType::ReturnStmt:
            return checkReturnStmt(std::static_pointer_cast<AST::ReturnStmtType>(node), env);
        case AST::NodeType::TemplateCall:
            return checkTemplateCall(std::static_pointer_cast<AST::TemplateCallType>(node), env);
        case AST::NodeType::CastExpr:
            return checkCastExpr(std::static_pointer_cast<AST::CastExprType>(node), env);
        case AST::NodeType::UnaryPrefix:
            return checkUnaryPrefix(std::static_pointer_cast<AST::UnaryPrefixType>(node), env);
        case AST::NodeType::UnaryPostFix:
            return checkUnaryPostfix(std::static_pointer_cast<AST::UnaryPostFixType>(node), env);
        case AST::NodeType::AwaitExpr:
            return checkAwaitExpr(std::static_pointer_cast<AST::AwaitExprType>(node), env);
        default:
            return std::make_shared<Type>(TypeKind::Any, "any");
    }
}

TypePtr TC::checkUnaryPrefix(std::shared_ptr<AST::UnaryPrefixType> expr, TypeEnvPtr env)
{
    TypePtr right = check(expr->assigne, env);
    if (expr->op == "!") return std::make_shared<Type>(TypeKind::Bool, "bool");
    
    // If the operator is not '!' it has to be '++' or '--'
    if (!compare(right, g_numty, env))
    {
        throw std::runtime_error(TypeError("'" + expr->op + "' can only be used on numbers", expr->assigne->token));
    }

    return right;
}

TypePtr TC::checkUnaryPostfix(std::shared_ptr<AST::UnaryPostFixType> expr, TypeEnvPtr env)
{
    TypePtr type = check(expr->assigne, env);
    // Unary postfix operators are only '++' and '--'

    if (!compare(type, g_numty, env))
    {
        throw std::runtime_error(TypeError("'" + expr->op + "' can only be used on numbers", expr->assigne->token));
    }

    return type;
}

TypePtr TC::checkForStmt(std::shared_ptr<AST::ForStmtType> forstmt, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    for (std::shared_ptr<AST::Stmt> decl : forstmt->declarations)
        check(decl, scope);

    for (std::shared_ptr<AST::Expr> cond : forstmt->conditions)
        check(cond, scope);

    for (std::shared_ptr<AST::Expr> update : forstmt->updates)
        check(update, scope);
    
    for (std::shared_ptr<AST::Stmt> stmt : forstmt->body)
        check(stmt, scope);

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkTemplateCall(std::shared_ptr<AST::TemplateCallType> call, TypeEnvPtr env)
{
    TypePtr caller = check(call->caller, env);

    if (
        caller->type == TypeKind::Class
        && caller->val->returntype
        && caller->val->returntype->type == TypeKind::Function
    )
    {
        TypePtr signature = std::make_shared<Type>(*caller->val->returntype);
        signature->val->params = {};

        if (call->templateArgs.empty())
        {
            throw std::runtime_error(CustomError("'function' template call requires one argument", "TemplateError", call->token));
        }

        std::string paramNames = "";
        signature->val->returntype = getType(call->templateArgs[0], env);

        for (size_t i = 1; i < call->templateArgs.size(); i++)
        {
            TypePtr type = getType(call->templateArgs[i], env);

            signature->val->params.push_back(std::make_shared<Parameter>("_arg", type, false));
            paramNames += ", " + type->name;
        }

        signature->name = "function<" + (signature->val->returntype ? signature->val->returntype->name : "any") + paramNames + ">";

        return signature;
    }
    else if (caller->val->sourcenode && caller->val->declenv)
    {
        TypeEnvPtr scope = std::make_shared<TypeEnv>(caller->val->declenv);

        if (call->templateArgs.size() != caller->val->templateparams.size())
        {
            throw std::runtime_error(
                CustomError("Template expects " + std::to_string(caller->val->templateparams.size()) + " template arguments, but " + std::to_string(call->templateArgs.size()) + " were provided", "TemplateError", call->token)
            );
        }

        for (size_t i = 0; i < caller->val->templateparams.size(); i++)
        {
            scope->declareVar(
                caller->val->templateparams[i]->identifier,
                getType(call->templateArgs[i], env),
                caller->val->templateparams[i]->token
            );
        }

        auto sourcenode = caller->val->sourcenode;

        if (sourcenode->kind == AST::NodeType::FunctionDeclaration)
        {
            auto fndecl = std::static_pointer_cast<AST::FunctionDeclarationType>(sourcenode);
            return checkFunction(fndecl, scope, true);
        }

        TypePtr result = check(sourcenode, scope);
        result->val->declenv = scope;

        return result;
    }

    return caller;
}

TypePtr TC::checkFunction(std::shared_ptr<AST::FunctionDeclarationType> fn, TypeEnvPtr env, bool templateProcessed)
{
    // `env` clone for template calls without changing `env`
    TypeEnvPtr declenv = std::make_shared<TypeEnv>(env);

    TypeEnvPtr scope = std::make_shared<TypeEnv>(declenv);

    if (!templateProcessed)
    {
        for (const auto& param : fn->templateparams)
        {
            scope->declareVar(
                param->identifier,
                std::make_shared<Type>(TypeKind::Any, "any"),
                param->token
            );
        }
    }

    std::unordered_set<std::string> usedParams;

    std::vector<std::shared_ptr<Parameter>> parameters;

    for (std::shared_ptr<AST::VarDeclarationType> param : fn->parameters)
    {
        if (usedParams.count(param->identifier))
        {
            throw std::runtime_error(CustomError("Duplicate parameter " + param->identifier, "ParameterError", param->token));
        }

        TypePtr type = getType(param->type, scope);

        usedParams.insert(param->identifier);
        scope->declareVar(param->identifier, param->staticType ? type : std::make_shared<Type>(TypeKind::Any, "any"), param->token);
        parameters.push_back(std::make_shared<Parameter>(param->identifier, type, (param->value && param->value->kind != AST::NodeType::UndefinedLiteral)));
    }

    m_currentret = fn->rettype ? getType(fn->rettype, scope) : std::make_shared<Type>(TypeKind::Any, "any");

    std::string functionIdent = "function<" + m_currentret->name;
    for (auto& param : parameters)
    {
        functionIdent += ", " + param->type->name;
    }

    functionIdent += ">";

    TypePtr type =
        std::make_shared<Type>(TypeKind::Function, functionIdent, std::make_shared<TypeVal>(parameters, m_currentret, fn->templateparams));
    
    type->val->sourcenode = fn;
    type->val->declenv = declenv;
    type->val->isAsync = fn->isAsync;

    env->declareVar(fn->name, type, fn->token);

    for (std::shared_ptr<AST::Stmt> stmt : fn->body)
    {
        check(stmt, scope);
    }

    m_currentret = nullptr;

    return type;
}

TypePtr TC::checkReturnStmt(std::shared_ptr<AST::ReturnStmtType> stmt, TypeEnvPtr env)
{
    if (!m_currentret)
    {
        throw std::runtime_error(TypeError("Did not expect return statement", stmt->token));
    }

    TypePtr rettype = check(stmt->val, env);

    if (!compare(m_currentret, rettype, env))
    {
        throw std::runtime_error(TypeError(rettype->name + " does not match expected return type, " + m_currentret->name, stmt->val->token));
    }

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkIfStmt(std::shared_ptr<AST::IfStmtType> stmt, TypeEnvPtr env)
{
    check(stmt->condition, env);

    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    for (std::shared_ptr<AST::Stmt> stmt : stmt->body)
    {
        check(stmt, scope);
    }

    if (stmt->hasElse)
    {
        TypeEnvPtr elsescope = std::make_shared<TypeEnv>(env);
        for (std::shared_ptr<AST::Stmt> stmt : stmt->elseStmt)
        {
            check(stmt, elsescope);
        }
    }

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkCastExpr(std::shared_ptr<AST::CastExprType> expr, TypeEnvPtr env)
{
    TypePtr left = check(expr->left, env);
    TypePtr type = getType(expr->type, env);

    if (!compare(left, type, env))
    {
        throw std::runtime_error(TypeError("Bad cast: " + type->name + " is not compatible with " + left->name, expr->token));
    }

    return type;
}

TypePtr TC::checkTernaryExpr(std::shared_ptr<AST::TernaryExprType> expr, TypeEnvPtr env)
{
    TypePtr cond = check(expr->cond, env);
    TypePtr cons = check(expr->cons, env);
    TypePtr alt = check(expr->alt, env);

    if (!compare(alt, cons, env))
    {
        throw std::runtime_error(TypeError("Ternary expression operands are incompatible: " + cons->name + " and " + alt->name, expr->token));
    }

    return cons;
}

TypePtr TC::checkMemberAssign(std::shared_ptr<AST::MemberAssignmentType> assign, TypeEnvPtr env)
{
    TypePtr obj = check(assign->object, env);
    TypePtr val = check(assign->newvalue, env);
    
    std::string key;
    if (!assign->computed && assign->property->kind == AST::NodeType::Identifier)
    {
        key = std::static_pointer_cast<AST::IdentifierType>(assign->property)->symbol;
        if (obj->val->props.find(key) != obj->val->props.end())
        {
            if (!compare(val, obj->val->props[key], env))
            {
                throw std::runtime_error(TypeError(val->name + " is not compatible with type " + obj->val->props[key]->name, assign->token));
            }
        }
        else if (obj->type == TypeKind::Module)
        {
            throw std::runtime_error(TypeError(obj->name + " does not have property " + key, assign->token));
        }
    }

    if (key.empty()) return std::make_shared<Type>(TypeKind::Any, "any");
    return val;
}

TypePtr TC::checkExportStmt(std::shared_ptr<AST::Stmt> stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx)
{
    if (stmt->kind == AST::NodeType::AssignmentExpr)
    {
        std::shared_ptr<AST::AssignmentExprType> assign = std::static_pointer_cast<AST::AssignmentExprType>(stmt);

        if (assign->assigne->kind != AST::NodeType::Identifier)
        {
            throw std::runtime_error(TypeError("Assignment exporting can only be used on identifiers", assign->token));
        }

        return check(std::make_shared<AST::VarDeclarationType>(assign->value, std::static_pointer_cast<AST::IdentifierType>(assign->assigne)->symbol), env, ctx);
    }

    return check(stmt, env, ctx);
}

TypePtr TC::checkClassDeclaration(std::shared_ptr<AST::ClassDefinitionType> cls, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    TypePtr thisobj = std::make_shared<Type>(TypeKind::Object, cls->name);

    scope->declareVar("this", thisobj, cls->token);
    
    if (cls->doesExtend) scope->declareVar("super", std::make_shared<Type>(TypeKind::Any, "any"), cls->extends->token);

    TypePtr type = std::make_shared<Type>(TypeKind::Class, "class");

    type->typeName = cls->name;
    if (cls->doesExtend)
    {
        type->parent = check(cls->extends, env);
    }

    checkClassInheritance(type, scope, thisobj);

    env->declareVar(cls->name, type, cls->token);

    for (std::shared_ptr<AST::Stmt> stmt : cls->body)
    {
        if (stmt->kind == AST::NodeType::VarDeclaration)
        {
            std::shared_ptr<AST::VarDeclarationType> decl = std::static_pointer_cast<AST::VarDeclarationType>(stmt);

            if (decl->staticType && decl->value && !compare(check(decl->value, scope), getType(decl->type, scope), scope))
            {
                throw std::runtime_error(TypeError(check(decl->value, scope)->name + " is not compatible with type " + getType(decl->type, scope)->name, decl->token));
            }

            thisobj->val->props[decl->identifier] = decl->staticType ? getType(decl->type, scope) : std::make_shared<Type>(TypeKind::Any, "any");
        }
        else if (stmt->kind == AST::NodeType::FunctionDeclaration)
        {
            std::shared_ptr<AST::FunctionDeclarationType> fn = std::static_pointer_cast<AST::FunctionDeclarationType>(stmt);

            thisobj->val->props[fn->name] = checkFunction(fn, scope);
        }
        else
        {
            check(stmt, scope);
        }
    }

    type->val->props = thisobj->val->props;

    return type;
}

void TC::checkClassInheritance(TypePtr cls, TypeEnvPtr env, TypePtr thisobj)
{
    if (cls->parent)
    {
        checkClassInheritance(cls->parent, env, thisobj);
        for (const auto& [key, type] : cls->parent->val->props)
        {
            thisobj->val->props[key] = type;
        }
    }
}

TypePtr TC::checkVarDecl(std::shared_ptr<AST::VarDeclarationType> decl, TypeEnvPtr env)
{
    if (decl->staticType && decl->value->kind != AST::NodeType::UndefinedLiteral)
    {
        TypePtr vartype = getType(decl->type, env);
        TypePtr assigntype = check(decl->value, env);

        if (!compare(vartype, assigntype, env))
        {
            throw std::runtime_error(TypeError("Cannot convert " + assigntype->name + " to " + vartype->name, decl->value->token));
        }
    }

    check(decl->value, env);
    env->declareVar(decl->identifier, decl->staticType ? getType(decl->type, env) : (decl->value != nullptr ? check(decl->value, env) : std::make_shared<Type>(TypeKind::Any, "any")), decl->token);

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkIdent(std::shared_ptr<AST::IdentifierType> ident, TypeEnvPtr env)
{
    return env->lookUp(ident->symbol, ident->token);
}

TypePtr TC::checkBinExpr(std::shared_ptr<AST::BinaryExprType> expr, TypeEnvPtr env)
{
    TypePtr left = check(expr->left, env);
    TypePtr right = check(expr->right, env);

    if (boolOps.count(expr->op))
    {
        return std::make_shared<Type>(TypeKind::Bool, "bool");
    }

    return left;
}

TypePtr TC::checkAssign(std::shared_ptr<AST::AssignmentExprType> assign, TypeEnvPtr env)
{
    TypePtr assigne = check(assign->assigne, env);

    if (assign->op != "=") return assigne;
    TypePtr value = check(assign->value, env);

    if (!compare(assigne, value, env))
    {
        throw std::runtime_error(TypeError("Cannot convert " + value->name + " to " + assigne->name, assign->value->token));
    }

    return value;
}

TypePtr TC::checkProbe(std::shared_ptr<AST::ProbeDeclarationType> prb, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);
    std::unordered_map<std::string, TypePtr> props;

    if (prb->doesExtend) checkProbeInheritance(check(prb->extends, env), scope);

    for (std::shared_ptr<AST::Stmt> stmt : prb->body)
    {
        if (stmt->kind == AST::NodeType::VarDeclaration)
        {
            std::shared_ptr<AST::VarDeclarationType> decl = std::static_pointer_cast<AST::VarDeclarationType>(stmt);

            props[decl->identifier] = checkVarDecl(decl, scope);
        } else if (stmt->kind == AST::NodeType::FunctionDeclaration) {
            std::shared_ptr<AST::FunctionDeclarationType> fn = std::static_pointer_cast<AST::FunctionDeclarationType>(stmt);
            
            props[fn->name] = checkFunction(fn, scope);
        } else
            check(stmt, scope);
    }

    return env->declareVar(prb->name, std::make_shared<Type>(TypeKind::Probe, "probe", std::make_shared<TypeVal>(props)), prb->token);
}

void TC::checkProbeInheritance(TypePtr prb, TypeEnvPtr env)
{
    if (prb->parent) checkProbeInheritance(prb->parent, env);

    env->massDeclare(prb->val->props);
}

TypePtr TC::checkCall(std::shared_ptr<AST::CallExprType> call, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);
    TypePtr fn = check(call->calee, scope);

    if (fn->type == TypeKind::Any) return std::make_shared<Type>(TypeKind::Any, "any");

    if (fn->type == TypeKind::Function)
    {
        TypeEnvPtr declenv = fn->val->declenv ? fn->val->declenv : scope;

        // First check that all required parameters are provided
        for (size_t i = 0; i < fn->val->params.size(); i++)
        {
            if (i >= call->args.size() && !fn->val->params[i]->hasDefault)
            {
                throw std::runtime_error(
                    TypeError("Function expects " + std::to_string(fn->val->params.size()) + " arguments, but " + std::to_string(call->args.size()) + " were provided", call->token)
                );
            }
        }

        // Then check the types of all arguments
        for (size_t i = 0; i < call->args.size(); i++)
        {
            TypePtr type = check(call->args[i], scope);

            if (
                fn->val->params.size() <= i
                || type->type == TypeKind::Any
                || !fn->val->params[i]
                || !fn->val->params[i]->type
                || fn->val->params[i]->type->type == TypeKind::Any
            ) continue;

            if (!compare(fn->val->params[i]->type, type, scope))
            {
                throw std::runtime_error(
                    TypeError("Function parameter " + std::to_string(i + 1) + " expects " + fn->val->params[i]->type->name + ", but got " + type->name + "\n", call->args[i]->token)
                );
            }
        }

        if (fn->val->isAsync)
        {
            return std::make_shared<Type>(TypeKind::Future, "future<" + (fn->val->returntype ? fn->val->returntype : std::make_shared<Type>(TypeKind::Any, "any"))->name + ">", std::make_shared<TypeVal>(false, fn->val->returntype ? fn->val->returntype : std::make_shared<Type>(TypeKind::Any, "any")));
        }

        return fn->val->returntype ? fn->val->returntype : std::make_shared<Type>(TypeKind::Any, "any");
    } else if (fn->type == TypeKind::Probe)
    {
        if (fn->val->props.find("run") == fn->val->props.end() || fn->val->props["run"]->type != TypeKind::Function)
        {
            throw std::runtime_error(TypeError("Probe has no 'run' method or it is not of type function", call->calee->token));
        }

        TypePtr run = fn->val->props["run"];

        TypeEnvPtr declenv = fn->val->declenv ? fn->val->declenv : scope;

        // First check that all required parameters are provided
        for (size_t i = 0; i < run->val->params.size(); i++)
        {
            if (i >= call->args.size() && !run->val->params[i]->hasDefault)
            {
                throw std::runtime_error(
                    TypeError("Probe expects " + std::to_string(run->val->params.size()) + " arguments, but " + std::to_string(call->args.size()) + " were provided", call->token)
                );
            }
        }

        // Then check the types of all arguments
        for (size_t i = 0; i < call->args.size(); i++)
        {
            TypePtr type = check(call->args[i], scope);

            if (
                run->val->params.size() <= i
                || type->type == TypeKind::Any
                || !run->val->params[i]
                || run->val->params[i]->type->type == TypeKind::Any
            ) continue;

            if (!compare(run->val->params[i]->type, type, scope))
            {
                throw std::runtime_error(
                    TypeError("Probe parameter " + std::to_string(i + 1) + " expects " + run->val->params[i]->type->name + ", but got " + type->name + "\n", call->args[i]->token)
                );
            }
        }

        return std::make_shared<Type>(TypeKind::Any, "any");
    } else
    {
        throw std::runtime_error(TypeError("Only function and probes can be called, but got " + fn->name, call->calee->token));
    }
}

TypePtr TC::checkObjectExpr(std::shared_ptr<AST::MapLiteralType> obj, TypeEnvPtr env)
{
    std::unordered_map<std::string, TypePtr> props;

    for (std::shared_ptr<AST::PropertyLiteralType> prop : obj->properties)
    {
        props[prop->key] = check(prop->val, env);
    }

    return std::make_shared<Type>(TypeKind::Object, "map", std::make_shared<TypeVal>(props));
}

TypePtr TC::checkMemberExpr(std::shared_ptr<AST::MemberExprType> expr, TypeEnvPtr env)
{
    TypePtr obj = check(expr->object, env);
    if (expr->property->kind == AST::NodeType::Identifier)
    {
        std::shared_ptr<AST::IdentifierType> ident = std::static_pointer_cast<AST::IdentifierType>(expr->property);

        if (obj->val->props.find(ident->symbol) != obj->val->props.end())
        {
            return obj->val->props[ident->symbol];
        }

        if (obj->type == TypeKind::Module)
        {
            throw std::runtime_error(TypeError("Object does not have property " + ident->symbol, ident->token));
        }
    }

    if (obj->type == TypeKind::Module)
    {
        throw std::runtime_error(TypeError("Object does not have that property", expr->property->token));
    }

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkArrayLiteral(std::shared_ptr<AST::ArrayLiteralType> array, TypeEnvPtr env)
{
    for (std::shared_ptr<AST::Expr> item : array->items)
    {
        check(item, env);
    }

    return std::make_shared<Type>(TypeKind::Array, "array");
}

TypePtr TC::checkArrowFunction(std::shared_ptr<AST::ArrowFunctionType> fn, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    std::vector<std::shared_ptr<Parameter>> parameters;

    for (std::shared_ptr<AST::VarDeclarationType> param : fn->params)
    {
        TypePtr type = param->staticType ? getType(param->type, env) : std::make_shared<Type>(TypeKind::Any, "any");
        scope->declareVar(param->identifier, type, param->token);
        parameters.push_back(std::make_shared<Parameter>(param->identifier, type, param->token, (param->value && param->value->kind != AST::NodeType::UndefinedLiteral)));
    }

    for (std::shared_ptr<AST::Stmt> stmt : fn->body)
    {
        check(stmt, scope);
    }

    TypePtr type = std::make_shared<Type>(TypeKind::Function, "function", std::make_shared<TypeVal>(parameters));
    type->val->returntype = g_anyty;
    return type;
}

TypePtr TC::checkImportStmt(std::shared_ptr<AST::ImportStmtType> stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx)
{
    std::unordered_map<std::string, TypePtr> stdlib;
    for (const auto& [key, pair] : g_stdlib)
        stdlib[key] = pair.second;

    if (stdlib.find(stmt->name) != stdlib.end())
    {
        TypePtr lib = stdlib[stmt->name];
        if (!stmt->hasMember)
        {
            return env->declareVar(stmt->customIdent ? stmt->ident : stmt->name, lib, stmt->token);
        }

        TypeEnvPtr tempenv = std::make_shared<TypeEnv>();
        tempenv->declareVar(stmt->name, lib, stmt->module->token);
        TypePtr member = checkMemberExpr(std::static_pointer_cast<AST::MemberExprType>(stmt->module), tempenv);

        return env->declareVar(stmt->customIdent ? stmt->ident : std::static_pointer_cast<AST::MemberExprType>(stmt->module)->lastProp, member, stmt->module->token);
    } else
    {
        if (ctx->modules.find(stmt->name) == ctx->modules.end())
        {
            throw std::runtime_error(CustomError("Module " + stmt->name + " not found", "ImportError", stmt->token));
        }

        fs::path path = ctx->modules[stmt->name];

        std::ifstream stream(path);
        std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        Parser parser;
        std::shared_ptr<Context> context = std::make_shared<Context>();
        context->file = file;
        context->filename = fs::absolute(path).string();
        context->modules = ctx->modules;

        std::shared_ptr<AST::ProgramType> program = parser.parse(file, context);
        m_context = context;
        std::unordered_map<std::string, TypePtr> exports = getExports(program, context);
        m_context = ctx;

        if (!stmt->hasMember)
        {
            return env->declareVar(stmt->customIdent ? stmt->ident : stmt->name, std::make_shared<Type>(TypeKind::Module, "module", std::make_shared<TypeVal>(exports)), stmt->token);
        }

        TypeEnvPtr tempenv = std::make_shared<TypeEnv>();
        tempenv->declareVar(stmt->name, std::make_shared<Type>(TypeKind::Module, "module", std::make_shared<TypeVal>(exports)), stmt->module->token);
        TypePtr member = checkMemberExpr(std::static_pointer_cast<AST::MemberExprType>(stmt->module), tempenv);

        return env->declareVar(stmt->customIdent ? stmt->ident : std::static_pointer_cast<AST::MemberExprType>(stmt->module)->lastProp, member, stmt->module->token);
    }

    return std::make_shared<Type>(TypeKind::Any, "module");
}

TypePtr TC::checkNewExpr(std::shared_ptr<AST::NewExprType> expr, TypeEnvPtr env)
{
    TypePtr cls = check(expr->constructor, env);

    if (cls->type == TypeKind::Any) return cls;

    if (cls->type != TypeKind::Class)
    {
        // Everything is allowed to be called as a class since something like `new num()` is allowed
        return cls;
    }

    if (cls->val->props.find("new") != cls->val->props.end())
    {
        TypePtr constructor = cls->val->props["new"];

        if (constructor->type == TypeKind::Function)
        {
            // Check that we have provided all required arguments
            size_t size = constructor->val->params.size();
            for (size_t i = 0; i < size; i++)
            {
                if (i >= expr->args.size() && !constructor->val->params[i]->hasDefault)
                {
                    throw std::runtime_error(CustomError("Constructor expects " + std::to_string(size) + " arguments, but only " + std::to_string(expr->args.size()) + " were provided", "ConstructError", expr->constructor->token));
                }
            }

            // Check the types of the arguments
            for (size_t i = 0; i < size; i++)
            {
                if (expr->args.size() <= i || !expr->args[i])
                    break;

                std::shared_ptr<Parameter> param = constructor->val->params[i];
                TypePtr arg = check(expr->args[i], env);

                if (!compare(arg, param->type, env))
                {
                    throw std::runtime_error(CustomError("Constructor expects '" + param->ident + "' to be of type " + param->type->name + " but it is of type " + arg->name, "ConstructorError", expr->args[i]->token));
                }
            }
        }
    }

    if (cls->val->returntype) return cls->val->returntype;
    TypeValPtr val = std::make_shared<TypeVal>(cls->val->props);

    TypePtr type = std::make_shared<Type>(TypeKind::Module, "object", val);
    type->isInstance = true;
    type->name = cls->typeName;
    type->parent = cls->parent;

    return type;
}

std::unordered_map<std::string, TypePtr> TC::getExports(std::shared_ptr<AST::ProgramType> program, std::shared_ptr<Context> ctx)
{
    TypeEnvPtr env = std::make_shared<TypeEnv>();
    std::unordered_map<std::string, TypePtr> exports;

    for (std::shared_ptr<AST::Stmt> stmt : program->body)
    {
        if (stmt->kind == AST::NodeType::ExportStmt)
        {
            std::shared_ptr<AST::ExportStmtType> exportstmt = std::static_pointer_cast<AST::ExportStmtType>(stmt);
            switch (exportstmt->exporting->kind)
            {
                case AST::NodeType::Identifier:
                    exports[std::static_pointer_cast<AST::IdentifierType>(exportstmt->exporting)->symbol] = check(exportstmt->exporting, env, ctx);
                    break;
                case AST::NodeType::AssignmentExpr:
                    if (std::static_pointer_cast<AST::AssignmentExprType>(exportstmt->exporting)->assigne->kind != AST::NodeType::Identifier)
                    {
                        throw std::runtime_error(TypeError("Only identifiers can be exported in assignment exporting", std::static_pointer_cast<AST::AssignmentExprType>(exportstmt->exporting)->assigne->token));
                    }
                    exports[std::static_pointer_cast<AST::IdentifierType>(std::static_pointer_cast<AST::AssignmentExprType>(exportstmt->exporting)->assigne)->symbol]
                        = check(std::static_pointer_cast<AST::AssignmentExprType>(exportstmt->exporting)->value, env, ctx);
                    break;
                case AST::NodeType::FunctionDeclaration:
                    exports[std::static_pointer_cast<AST::FunctionDeclarationType>(exportstmt->exporting)->name]
                        = check(std::static_pointer_cast<AST::FunctionDeclarationType>(exportstmt->exporting), env, ctx);
                    break;
                case AST::NodeType::ProbeDeclaration:
                    exports[std::static_pointer_cast<AST::ProbeDeclarationType>(exportstmt->exporting)->name]
                        = check(std::static_pointer_cast<AST::ProbeDeclarationType>(exportstmt->exporting), env, ctx);
                    break;
                case AST::NodeType::ClassDefinition:
                    exports[std::static_pointer_cast<AST::ClassDefinitionType>(exportstmt->exporting)->name]
                        = check(std::static_pointer_cast<AST::ClassDefinitionType>(exportstmt->exporting), env, ctx);
                    break;
                default:
                    throw std::runtime_error(TypeError("Unknown export type", exportstmt->exporting->token));
            }
        } else check(stmt, env, ctx);
    }

    return exports;
}

TypePtr TC::checkAwaitExpr(std::shared_ptr<AST::AwaitExprType> expr, TypeEnvPtr env)
{
    TypePtr type = check(expr->caller, env);
    if (!compare(type, std::make_shared<Type>(TypeKind::Future, "future"), env))
    {
        throw std::runtime_error(TypeError("Cannot await a value that is not a future", expr->caller->token));
    }

    return type->val->futureVal;
}

TypePtr TC::getType(std::shared_ptr<AST::Expr> name, TypeEnvPtr env)
{
    if (!name) return std::make_shared<Type>(TypeKind::Any, "any");

    if (name->kind == AST::NodeType::Identifier)
    {
        std::shared_ptr<AST::IdentifierType> ident = std::static_pointer_cast<AST::IdentifierType>(name);

        if (ident->symbol == "str")
            return std::make_shared<Type>(TypeKind::String, "string");
        else if (ident->symbol == "num")
            return std::make_shared<Type>(TypeKind::Number, "number");
        else if (ident->symbol == "bool")
            return std::make_shared<Type>(TypeKind::Bool, "bool");
        else if (ident->symbol == "map")
            return std::make_shared<Type>(TypeKind::Object, "map");
        else if (ident->symbol == "function")
            return std::make_shared<Type>(TypeKind::Function, "function");
        else if (ident->symbol == "array")
            return std::make_shared<Type>(TypeKind::Array, "array");
	    else if (ident->symbol == "any")
	        return std::make_shared<Type>(TypeKind::Any, "any");
    }

    if (name->kind == AST::NodeType::MapLiteral)
    {
        auto map = std::static_pointer_cast<AST::MapLiteralType>(name);

        auto type = std::make_shared<Type>(TypeKind::Module, "object");
        type->isInstance = true;

        for (auto& prop : map->properties)
        {
            type->val->props[prop->key] = getType(prop->val, env);
        }

        return type;
    }

    TypePtr type = std::make_shared<Type>(check(name, env));

    if (type->type == TypeKind::Class)
    {
        type->type = TypeKind::Module;
        type->isInstance = true;
        type->name = type->typeName;
    }

    return type;
}


bool TC::compare(TypePtr left, TypePtr right, TypeEnvPtr env)
{
    if (left->type == TypeKind::Any || right->type == TypeKind::Any)
        return true;
    else
    {
        if (right->isInstance)
        {
            if (right->val->props == left->val->props) return true;

            TypePtr current = right;
            while ((current = current->parent))
            {
                if (current->val->props == left->val->props) return true;
            }

            return false;
        }

        if (left->isInstance)
        {
            if (left->val->props == right->val->props) return true;

            TypePtr current = left;
            while ((current = current->parent))
            {
                if (current->val->props == right->val->props) return true;
            }

            return false;
        }

        if (
            right->type == TypeKind::Function
            && left->type == TypeKind::Function
            && right->val->returntype
            && left->val->returntype
        )
        {
            if (right->val->params.size() != left->val->params.size())
            {
                return false;
            }

            if (left->val->returntype && right->val->returntype && !compare(left->val->returntype, right->val->returntype, env))
            {
                return false;
            }

            size_t size = right->val->params.size();

            for (size_t i = 0; i < size; i++)
            {
                if (!compare(right->val->params[i]->type, left->val->params[i]->type, env))
                {
                    return false;
                }
            }

            return true;
        }

        return (left->type == right->type);
    }
}
