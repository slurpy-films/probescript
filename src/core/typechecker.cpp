#include "typechecker.hpp"

TypeEnv::TypeEnv(std::shared_ptr<TypeEnv> parent)
{
    if (parent) m_parent = parent;

    for (const auto& [key, pair] : g_globals)
        m_variables[key] = pair.second;
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

void TC::checkProgram(ProgramType* program, TypeEnvPtr env, std::shared_ptr<Context> ctx)
{
    m_context = ctx;
    for (Stmt* stmt : program->body)
    {
        check(stmt, env, ctx);
    }
}

TypePtr TC::check(Stmt* node, TypeEnvPtr env, std::shared_ptr<Context> ctx)
{
    if (!node) return std::make_shared<Type>(TypeKind::Any, "any");
    switch (node->kind)
    {
        case NodeType::NumericLiteral:
            return std::make_shared<Type>(TypeKind::Number, "number");
        case NodeType::StringLiteral:
            return std::make_shared<Type>(TypeKind::String, "string");
        case NodeType::BoolLiteral:
            return std::make_shared<Type>(TypeKind::Bool, "bool");
        case NodeType::VarDeclaration:
            return checkVarDecl(static_cast<VarDeclarationType*>(node), env);
        case NodeType::AssignmentExpr:
            return checkAssign(static_cast<AssignmentExprType*>(node), env);
        case NodeType::Identifier:
            return checkIdent(static_cast<IdentifierType*>(node), env);
        case NodeType::FunctionDeclaration:
            return checkFunction(static_cast<FunctionDeclarationType*>(node), env);
        case NodeType::ProbeDeclaration:
            return checkProbe(static_cast<ProbeDeclarationType*>(node), env);
        case NodeType::CallExpr:
            return checkCall(static_cast<CallExprType*>(node), env);
        case NodeType::MapLiteral:
            return checkObjectExpr(static_cast<MapLiteralType*>(node), env);
        case NodeType::MemberExpr:
            return checkMemberExpr(static_cast<MemberExprType*>(node), env);
        case NodeType::ImportStmt:
            return checkImportStmt(static_cast<ImportStmtType*>(node), env, ctx);
        case NodeType::ExportStmt:
            return checkExportStmt(static_cast<ExportStmtType*>(node)->exporting, env, ctx);
        case NodeType::BinaryExpr:
            return checkBinExpr(static_cast<BinaryExprType*>(node), env);
        case NodeType::ClassDefinition:
            return checkClassDeclaration(static_cast<ClassDefinitionType*>(node), env);
        case NodeType::NewExpr:
            return checkNewExpr(static_cast<NewExprType*>(node), env);
        case NodeType::IfStmt:
            return checkIfStmt(static_cast<IfStmtType*>(node), env);
        case NodeType::ForStmt:
            return checkForStmt(static_cast<ForStmtType*>(node), env);
        case NodeType::MemberAssignment:
            return checkMemberAssign(static_cast<MemberAssignmentType*>(node), env);
        case NodeType::ArrayLiteral:
            return checkArrayLiteral(static_cast<ArrayLiteralType*>(node), env);
        case NodeType::ArrowFunction:
            return checkArrowFunction(static_cast<ArrowFunctionType*>(node), env);
        case NodeType::TernaryExpr:
            return checkTernaryExpr(static_cast<TernaryExprType*>(node), env);
        case NodeType::ReturnStmt:
            return checkReturnStmt(static_cast<ReturnStmtType*>(node), env);
        case NodeType::TemplateCall:
            return checkTemplateCall(static_cast<TemplateCallType*>(node), env);
        case NodeType::CastExpr:
            return checkCastExpr(static_cast<CastExprType*>(node), env);
        case NodeType::UnaryPrefix:
            return checkUnaryPrefix(static_cast<UnaryPrefixType*>(node), env);
        default:
            return std::make_shared<Type>(TypeKind::Any, "any");
    }
}

TypePtr TC::checkUnaryPrefix(UnaryPrefixType* expr, TypeEnvPtr env)
{
    TypePtr right = check(expr->assigne, env);
    if (expr->op == "!") return std::make_shared<Type>(TypeKind::Bool, "bool");
    else return right;
}

TypePtr TC::checkForStmt(ForStmtType* forstmt, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    for (Stmt* decl : forstmt->declarations)
        check(decl, scope);

    for (Expr* cond : forstmt->conditions)
        check(cond, scope);

    for (Expr* update : forstmt->updates)
        check(update, scope);
    
    for (Stmt* stmt : forstmt->body)
        check(stmt, scope);

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkTemplateCall(TemplateCallType* call, TypeEnvPtr env)
{
    TypePtr caller = check(call->caller, env);

    for (size_t i = 0; i < caller->val->templateparams.size(); i++)
    {
        env->declareVar(caller->val->templateparams[i]->identifier, (call->templateArgs.size() > i ? getType(call->templateArgs[i], env) : std::make_shared<Type>(TypeKind::Any, "any")), caller->val->templateparams[i]->token);
    }

    if (caller->type == TypeKind::Function)
    {
        if (
            caller->val->returntype
            && caller->val->returntype->templateSub
            && caller->val->returntype->val->sourcenode
        )
        {
            VarDeclarationType* src = caller->val->returntype->val->sourcenode;
            TypePtr type = env->lookUp(src->identifier, src->token);
            
            caller->val->returntype = type;
        }
    }

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

        std::string paramNames = ", ";
        signature->val->returntype = getType(call->templateArgs[0], env);

        for (size_t i = 1; i < call->templateArgs.size(); i++)
        {
            signature->val->params.push_back(new VarDeclarationType(new UndefinedLiteralType(), "_arg", call->templateArgs[i]));
            paramNames += getType(call->templateArgs[i], env)->name;
            if (i + 1 < call->templateArgs.size())
            {
                paramNames += ", ";
            }
        }

        signature->name = "function<" + (signature->val->returntype ? signature->val->returntype->name : "any") + paramNames + ">";

        return signature;
    }

    return caller;
}

TypePtr TC::checkReturnStmt(ReturnStmtType* stmt, TypeEnvPtr env)
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

TypePtr TC::checkIfStmt(IfStmtType* stmt, TypeEnvPtr env)
{
    check(stmt->condition, env);

    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    for (Stmt* stmt : stmt->body)
    {
        check(stmt, scope);
    }

    if (stmt->hasElse){
        TypeEnvPtr elsescope = std::make_shared<TypeEnv>(env);
        for (Stmt* stmt : stmt->elseStmt)
        {
            check(stmt, elsescope);
        }
    }

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkCastExpr(CastExprType* expr, TypeEnvPtr env)
{
    TypePtr left = check(expr->left, env);
    TypePtr type = getType(expr->type, env);

    if (!compare(left, type, env))
    {
        throw std::runtime_error(TypeError("Bad cast: " + type->name + " is not compatible with " + left->name, expr->token));
    }

    return type;
}

TypePtr TC::checkTernaryExpr(TernaryExprType* expr, TypeEnvPtr env)
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

TypePtr TC::checkMemberAssign(MemberAssignmentType* assign, TypeEnvPtr env)
{
    TypePtr obj = check(assign->object, env);
    TypePtr val = check(assign->newvalue, env);
    
    std::string key;
    if (!assign->computed && assign->property->kind == NodeType::Identifier)
    {
        key = static_cast<IdentifierType*>(assign->property)->symbol;
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

TypePtr TC::checkExportStmt(Stmt* stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx)
{
    if (stmt->kind == NodeType::AssignmentExpr)
    {
        AssignmentExprType* assign = static_cast<AssignmentExprType*>(stmt);

        if (assign->assigne->kind != NodeType::Identifier)
        {
            throw std::runtime_error(TypeError("Assignment exporting can only be used on identifiers", assign->token));
        }

        return check(new VarDeclarationType(assign->value, static_cast<IdentifierType*>(assign->assigne)->symbol), env, ctx);
    }

    return check(stmt, env, ctx);
}

TypePtr TC::checkClassDeclaration(ClassDefinitionType* cls, TypeEnvPtr env)
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

    for (Stmt* stmt : cls->body)
    {
        if (stmt->kind == NodeType::VarDeclaration)
        {
            VarDeclarationType* decl = static_cast<VarDeclarationType*>(stmt);

            if (decl->staticType && decl->value && !compare(check(decl->value, scope), getType(decl->type, scope), scope))
            {
                throw std::runtime_error(TypeError(check(decl->value, scope)->name + " is not compatible with type " + getType(decl->type, scope)->name, decl->token));
            }

            thisobj->val->props[decl->identifier] = decl->staticType ? getType(decl->type, scope) : std::make_shared<Type>(TypeKind::Any, "any");
        }
        else if (stmt->kind == NodeType::FunctionDeclaration)
        {
            FunctionDeclarationType* fn = static_cast<FunctionDeclarationType*>(stmt);

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

TypePtr TC::checkVarDecl(VarDeclarationType* decl, TypeEnvPtr env)
{
    if (decl->staticType && decl->value->kind != NodeType::UndefinedLiteral)
    {
        TypePtr vartype = getType(decl->type, env);
        TypePtr assigntype = check(decl->value, env);

        if (!compare(vartype, assigntype, env))
        {
            throw std::runtime_error(TypeError("Cannot convert " + assigntype->name + " to " + vartype->name, decl->value->token));
        }
    }

    check(decl->value, env);
    env->declareVar(decl->identifier, decl->staticType ? getType(decl->type, env) : std::make_shared<Type>(TypeKind::Any, "any"), decl->token);

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkIdent(IdentifierType* ident, TypeEnvPtr env)
{
    return env->lookUp(ident->symbol, ident->token);
}

TypePtr TC::checkBinExpr(BinaryExprType* expr, TypeEnvPtr env)
{
    TypePtr left = check(expr->left, env);
    TypePtr right = check(expr->right, env);

    if (boolOps.count(expr->op))
    {
        return std::make_shared<Type>(TypeKind::Bool, "bool");
    }

    return left;
}

TypePtr TC::checkAssign(AssignmentExprType* assign, TypeEnvPtr env)
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

TypePtr TC::checkProbe(ProbeDeclarationType* prb, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);
    std::unordered_map<std::string, TypePtr> props;

    if (prb->doesExtend) checkProbeInheritance(check(prb->extends, env), scope);

    for (Stmt* stmt : prb->body)
    {
        if (stmt->kind == NodeType::VarDeclaration)
        {
            VarDeclarationType* decl = static_cast<VarDeclarationType*>(stmt);

            props[decl->identifier] = checkVarDecl(decl, scope);
        } else if (stmt->kind == NodeType::FunctionDeclaration) {
            FunctionDeclarationType* fn = static_cast<FunctionDeclarationType*>(stmt);
            
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

TypePtr TC::checkFunction(FunctionDeclarationType* fn, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    for (VarDeclarationType* param : fn->templateparams)
    {
        TypePtr type = std::make_shared<Type>(TypeKind::Any, "any");
        type->templateSub = true;
        type->val->sourcenode = param;

        scope->declareVar(param->identifier, type, param->token);
    }

    std::unordered_set<std::string> usedParams;

    for (VarDeclarationType* param : fn->parameters)
    {
        if (usedParams.count(param->identifier))
        {
            throw std::runtime_error(CustomError("Duplicate parameter " + param->identifier, "ParameterError", param->token));
        }

        usedParams.insert(param->identifier);
        scope->declareVar(param->identifier, param->staticType ? getType(param->type, scope) : std::make_shared<Type>(TypeKind::Any, "any"), param->token);
    }

    m_currentret = fn->rettype ? getType(fn->rettype, scope) : std::make_shared<Type>(TypeKind::Any, "any");

    TypePtr type =
        std::make_shared<Type>(TypeKind::Function, "function", std::make_shared<TypeVal>(fn->parameters, fn->rettype ? getType(fn->rettype, scope) : std::make_shared<Type>(TypeKind::Any, "any"), fn->templateparams));

    env->declareVar(fn->name, type, fn->token);
    
    for (Stmt* stmt : fn->body)
    {
        check(stmt, scope);
    }

    m_currentret = nullptr;

    return type;
}

TypePtr TC::checkCall(CallExprType* call, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);
    TypePtr fn = check(call->calee, scope);


    if (fn->type == TypeKind::Any) return std::make_shared<Type>(TypeKind::Any, "any");

    if (fn->type == TypeKind::Function)
    {
        // First check that all required parameters are provided
        for (size_t i = 0; i < fn->val->params.size(); i++)
        {
            if (i >= call->args.size() && fn->val->params[i]->value && fn->val->params[i]->value->kind == NodeType::UndefinedLiteral)
            {
                throw std::runtime_error(
                    TypeError("Function expects " + std::to_string(fn->val->params.size()) + " arguments, but " + std::to_string(call->args.size()) + " were provided", call->token)
                );
            }
        }

        // Then check the type of all arguments
        for (size_t i = 0; i < call->args.size(); i++)
        {
            TypePtr type = check(call->args[i], scope);

            if (
                fn->val->params.size() <= i || type->type == TypeKind::Any || getType(fn->val->params[i]->type, scope)->type == TypeKind::Any
            ) continue;

            if (!compare(getType(fn->val->params[i]->type, scope), type, scope))
            {
                throw std::runtime_error(
                    TypeError("Function parameter " + std::to_string(i + 1) + " expects " + getType(fn->val->params[i]->type, scope)->name + ", but got " + type->name + "\n", call->args[i]->token)
                );
            }
        }

        return fn->val->returntype ? fn->val->returntype : std::make_shared<Type>(TypeKind::Any, "any");
    } else if (fn->type == TypeKind::Probe)
    {
        if (fn->val->props.find("run") == fn->val->props.end() || fn->val->props["run"]->type != TypeKind::Function)
        {
            throw std::runtime_error(TypeError("Probe has no 'run' method or it is not of type function", call->calee->token));
        }

        TypePtr run = fn->val->props["run"];

        // First check that all required parameters are provided
        for (size_t i = 0; i < run->val->params.size(); i++)
        {
            if (i >= call->args.size() && run->val->params[i]->value && run->val->params[i]->value->kind == NodeType::UndefinedLiteral)
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
                run->val->params.size() <= i || type->type == TypeKind::Any || getType(run->val->params[i]->type, scope)->type == TypeKind::Any
            ) continue;

            if (!compare(getType(run->val->params[i]->type, scope), type, scope))
            {
                throw std::runtime_error(
                    TypeError("Probe parameter " + std::to_string(i + 1) + " expects " + getType(run->val->params[i]->type, scope)->name + ", but got " + type->name + "\n", call->args[i]->token)
                );
            }
        }

        return std::make_shared<Type>(TypeKind::Any, "any");
    } else
    {
        throw std::runtime_error(TypeError("Only function and probes can be called, but got " + fn->name, call->calee->token));
    }
}

TypePtr TC::checkObjectExpr(MapLiteralType* obj, TypeEnvPtr env)
{
    std::unordered_map<std::string, TypePtr> props;

    for (PropertyLiteralType* prop : obj->properties)
    {
        props[prop->key] = check(prop->val, env);
    }

    return std::make_shared<Type>(TypeKind::Object, "map", std::make_shared<TypeVal>(props));
}

TypePtr TC::checkMemberExpr(MemberExprType* expr, TypeEnvPtr env)
{
    TypePtr obj = check(expr->object, env);
    if (expr->property->kind == NodeType::Identifier)
    {
        IdentifierType* ident = static_cast<IdentifierType*>(expr->property);

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

TypePtr TC::checkArrayLiteral(ArrayLiteralType* array, TypeEnvPtr env)
{
    for (Expr* item : array->items)
    {
        check(item, env);
    }

    return std::make_shared<Type>(TypeKind::Array, "array");
}

TypePtr TC::checkArrowFunction(ArrowFunctionType* fn, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    for (VarDeclarationType* param : fn->params)
    {
        scope->declareVar(param->identifier, param->staticType ? getType(param->type, env) : std::make_shared<Type>(TypeKind::Any, "any"), param->token);
    }

    for (Stmt* stmt : fn->body)
    {
        check(stmt, scope);
    }

    return std::make_shared<Type>(TypeKind::Function, "function", std::make_shared<TypeVal>(fn->params));
}

TypePtr TC::checkImportStmt(ImportStmtType* stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx)
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
        TypePtr member = checkMemberExpr(static_cast<MemberExprType*>(stmt->module), tempenv);

        return env->declareVar(stmt->customIdent ? stmt->ident : static_cast<MemberExprType*>(stmt->module)->lastProp, member, stmt->module->token);
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

        ProgramType* program = parser.parse(file, context);
        m_context = context;
        std::unordered_map<std::string, TypePtr> exports = getExports(program, context);
        m_context = ctx;

        if (!stmt->hasMember)
        {
            return env->declareVar(stmt->customIdent ? stmt->ident : stmt->name, std::make_shared<Type>(TypeKind::Module, "module", std::make_shared<TypeVal>(exports)), stmt->token);
        }

        TypeEnvPtr tempenv = std::make_shared<TypeEnv>();
        tempenv->declareVar(stmt->name, std::make_shared<Type>(TypeKind::Module, "module", std::make_shared<TypeVal>(exports)), stmt->module->token);
        TypePtr member = checkMemberExpr(static_cast<MemberExprType*>(stmt->module), tempenv);

        return env->declareVar(stmt->customIdent ? stmt->ident : static_cast<MemberExprType*>(stmt->module)->lastProp, member, stmt->module->token);
    }

    return std::make_shared<Type>(TypeKind::Any, "module");
}

TypePtr TC::checkNewExpr(NewExprType* expr, TypeEnvPtr env)
{
    TypePtr cls = check(expr->constructor, env);

    if (cls->type == TypeKind::Any) return cls;

    if (cls->type != TypeKind::Class)
    {
        throw std::runtime_error(TypeError("Expected constructor to be of type class, got " + cls->name, expr->constructor->token));
    }

    if (cls->val->returntype) return cls->val->returntype;
    TypeValPtr val = std::make_shared<TypeVal>(cls->val->props);

    TypePtr type = std::make_shared<Type>(TypeKind::Module, "object", val);
    type->isInstance = true;
    type->name = cls->typeName;
    type->parent = cls->parent;

    return type;
}

std::unordered_map<std::string, TypePtr> TC::getExports(ProgramType* program, std::shared_ptr<Context> ctx)
{
    TypeEnvPtr env = std::make_shared<TypeEnv>();
    std::unordered_map<std::string, TypePtr> exports;

    for (Stmt* stmt : program->body)
    {
        if (stmt->kind == NodeType::ExportStmt)
        {
            ExportStmtType* exportstmt = static_cast<ExportStmtType*>(stmt);
            switch (exportstmt->exporting->kind)
            {
                case NodeType::Identifier:
                    exports[static_cast<IdentifierType*>(exportstmt->exporting)->symbol] = check(exportstmt->exporting, env, ctx);
                    break;
                case NodeType::AssignmentExpr:
                    if (static_cast<AssignmentExprType*>(exportstmt->exporting)->assigne->kind != NodeType::Identifier)
                    {
                        throw std::runtime_error(TypeError("Only identifiers can be exported in assignment exporting", static_cast<AssignmentExprType*>(exportstmt->exporting)->assigne->token));
                    }
                    exports[static_cast<IdentifierType*>(static_cast<AssignmentExprType*>(exportstmt->exporting)->assigne)->symbol]
                        = check(static_cast<AssignmentExprType*>(exportstmt->exporting)->value, env, ctx);
                    break;
                case NodeType::FunctionDeclaration:
                    exports[static_cast<FunctionDeclarationType*>(exportstmt->exporting)->name]
                        = check(static_cast<FunctionDeclarationType*>(exportstmt->exporting), env, ctx);
                    break;
                case NodeType::ProbeDeclaration:
                    exports[static_cast<ProbeDeclarationType*>(exportstmt->exporting)->name]
                        = check(static_cast<ProbeDeclarationType*>(exportstmt->exporting), env, ctx);
                    break;
                case NodeType::ClassDefinition:
                    exports[static_cast<ClassDefinitionType*>(exportstmt->exporting)->name]
                        = check(static_cast<ClassDefinitionType*>(exportstmt->exporting), env, ctx);
                    break;
                default:
                    throw std::runtime_error(TypeError("Unknown export type", exportstmt->exporting->token));
            }
        } else check(stmt, env, ctx);
    }

    return exports;
}

TypePtr TC::getType(Expr* name, TypeEnvPtr env)
{
    if (!name) return std::make_shared<Type>(TypeKind::Any, "any");

    if (name->kind == NodeType::Identifier)
    {
        TypePtr type;
        IdentifierType* ident = static_cast<IdentifierType*>(name);

        if (ident->symbol == "str")
            type = std::make_shared<Type>(TypeKind::String, "string");
        else if (ident->symbol == "num")
            type = std::make_shared<Type>(TypeKind::Number, "number");
        else if (ident->symbol == "bool")
            type = std::make_shared<Type>(TypeKind::Bool, "bool");
        else if (ident->symbol == "map")
            type = std::make_shared<Type>(TypeKind::Object, "map");
        else if (ident->symbol == "function")
            type = std::make_shared<Type>(TypeKind::Function, "function");
        else if (ident->symbol == "array")
            type = std::make_shared<Type>(TypeKind::Array, "array");
        
        if (type) return type;
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

        if (right->type == TypeKind::Function && left->type == TypeKind::Function)
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
                if (!compare(getType(right->val->params[i]->type, env), getType(left->val->params[i]->type, env), env))
                {
                    return false;
                }
            }

            return true;
        }

        return (left->type == right->type);
    }
}