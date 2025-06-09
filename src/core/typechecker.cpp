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

TypePtr TypeEnv::declareVar(std::string name, TypePtr type)
{
    if (m_variables.find(name) != m_variables.end())
    {
        std::cerr << ManualError("Variable " + name + " is already defined", "RedefinitionError");
        exit(1);
    }

    m_variables[name] = type;

    return type;
}

TypePtr TypeEnv::lookUp(std::string name)
{
    if (m_variables.find(name) != m_variables.end())
        return m_variables[name];
    else if (m_parent)
        return m_parent->lookUp(name);
    else
    {
        std::cerr << ManualError("Variable " + name + " is not defined", "ReferenceError");
        exit(1);
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
        default:
            return std::make_shared<Type>(TypeKind::Any, "any");
    }
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
        env->declareVar(caller->val->templateparams[i]->identifier, (call->templateArgs.size() > i ? getType(call->templateArgs[i], env) : std::make_shared<Type>(TypeKind::Any, "any")));
    }

    if (caller->type == TypeKind::Function)
    {
        if (caller->val->returntype && caller->val->returntype->templateSub && caller->val->returntype->val->sourcenode)
        {
            VarDeclarationType* src = caller->val->returntype->val->sourcenode;
            TypePtr type = env->lookUp(src->identifier);
            

            caller->val->returntype = type;
        }
    }

    return caller;
}

TypePtr TC::checkReturnStmt(ReturnStmtType* stmt, TypeEnvPtr env)
{
    if (!m_currentret)
    {
        std::cerr << TypeError("Did not expect return statment", stmt->token, m_context);
        exit(1);
    }

    TypePtr rettype = check(stmt->stmt, env);

    if (!compare(m_currentret, rettype, env))
    {
        std::cerr << TypeError(rettype->name + " does not match expected return type, " + m_currentret->name, stmt->token, m_context);
        exit(1);
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

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkTernaryExpr(TernaryExprType* expr, TypeEnvPtr env)
{
    TypePtr cond = check(expr->cond, env);
    TypePtr cons = check(expr->cons, env);
    TypePtr alt = check(expr->alt, env);

    if (!compare(alt, cons, env))
    {
        std::cerr << TypeError("Ternary expression operands are incompatible: " + cons->name + " and " + alt->name, expr->token, m_context);
        exit(1);
    }

    return cons;
}

TypePtr TC::checkMemberAssign(MemberAssignmentType* assign, TypeEnvPtr env)
{
    TypePtr obj = check(assign->object, env);
    
    std::string key;
    if (!assign->computed)
    {
        key = static_cast<IdentifierType*>(assign->property)->symbol;
    }

    if (key.empty()) return std::make_shared<Type>(TypeKind::Any, "any");
    return obj->val->props[key] = check(assign->newvalue, env);
}

TypePtr TC::checkExportStmt(Stmt* stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx)
{
    if (stmt->kind == NodeType::AssignmentExpr)
    {
        AssignmentExprType* assign = static_cast<AssignmentExprType*>(stmt);

        if (assign->assigne->kind != NodeType::Identifier)
        {
            std::cerr << TypeError("Assignment exporting can only be used on identifiers", assign->token, m_context);
            exit(1);
        }

        return check(new VarDeclarationType(assign->value, static_cast<IdentifierType*>(assign->assigne)->symbol), env, ctx);
    }

    return check(stmt, env, ctx);
}

TypePtr TC::checkClassDeclaration(ClassDefinitionType* cls, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    scope->declareVar("this", std::make_shared<Type>(TypeKind::Object, cls->name));
    
    if (cls->doesExtend) scope->declareVar("super", std::make_shared<Type>(TypeKind::Any, "any"));

    for (Stmt* stmt : cls->body)
    {
        if (stmt->kind == NodeType::AssignmentExpr)
        {
            AssignmentExprType* assign = static_cast<AssignmentExprType*>(stmt);
            if (assign->assigne->kind != NodeType::Identifier)
            {
                std::cerr << TypeError("Only identifier members are allowed in classes", assign->token, m_context);
                exit(1);
            }

            scope->declareVar(static_cast<IdentifierType*>(assign->assigne)->symbol, check(assign->value, scope));
        } else
            check(stmt, scope);
    }

    TypePtr type = std::make_shared<Type>(TypeKind::Class, "class", std::make_shared<TypeVal>(scope->getVars()));
    type->typeName = cls->name;
    if (cls->doesExtend)
    {
        type->parent = check(cls->extends, env);
    }

    return env->declareVar(cls->name, type);
}

TypePtr TC::checkVarDecl(VarDeclarationType* decl, TypeEnvPtr env)
{
    if (decl->staticType && decl->value->kind != NodeType::UndefinedLiteral)
    {
        TypePtr vartype = getType(decl->type, env);
        TypePtr assigntype = check(decl->value, env);

        if (!compare(vartype, assigntype, env))
        {
            std::cerr << TypeError("Cannot convert " + assigntype->name + " to " + vartype->name, decl->token, m_context);
            exit(1);
        }
    }

    check(decl->value, env);
    env->declareVar(decl->identifier, decl->staticType ? getType(decl->type, env) : std::make_shared<Type>(TypeKind::Any, "any"));

    return std::make_shared<Type>(TypeKind::Any, "any");
}

TypePtr TC::checkIdent(IdentifierType* ident, TypeEnvPtr env)
{
    return env->lookUp(ident->symbol);
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
        std::cerr << TypeError("Cannot convert " + value->name + " to " + assigne->name, assign->value->token, m_context);
        exit(1);
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

    return env->declareVar(prb->name, std::make_shared<Type>(TypeKind::Probe, "probe", std::make_shared<TypeVal>(props)));
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

        scope->declareVar(param->identifier, type);
    }

    for (VarDeclarationType* param : fn->parameters)
    {
        scope->declareVar(param->identifier, param->staticType ? getType(param->type, scope) : std::make_shared<Type>(TypeKind::Any, "any"));
    }

    m_currentret = fn->rettype ? getType(fn->rettype, scope) : std::make_shared<Type>(TypeKind::Any, "any");

    for (Stmt* stmt : fn->body)
    {
        check(stmt, scope);
    }

    m_currentret = nullptr;

    TypePtr type =
        std::make_shared<Type>(TypeKind::Function, "function", std::make_shared<TypeVal>(fn->parameters, fn->rettype ? getType(fn->rettype, scope) : std::make_shared<Type>(TypeKind::Any, "any"), fn->templateparams));

    return env->declareVar(fn->name, type);
}

TypePtr TC::checkCall(CallExprType* call, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);
    TypePtr fn = check(call->calee, scope);


    if (fn->type == TypeKind::Any) return std::make_shared<Type>(TypeKind::Any, "any");

    if (fn->type == TypeKind::Function)
    {
        for (size_t i = 0; i < call->args.size(); i++)
        {
            TypePtr type = check(call->args[i], scope);

            if (
                fn->val->params.size() <= i || type->type == TypeKind::Any || getType(fn->val->params[i]->type, scope)->type == TypeKind::Any
            ) continue;

            if (!compare(getType(fn->val->params[i]->type, scope), type, scope))
            {
                std::cerr
                    << TypeError("Function parameter " + std::to_string(i + 1) + " expects " + getType(fn->val->params[i]->type, scope)->name + ", but got " + type->name + "\n", call->args[i]->token, m_context);
                exit(1);
            }
        }

        return fn->val->returntype ? fn->val->returntype : std::make_shared<Type>(TypeKind::Any, "any");
    } else if (fn->type == TypeKind::Probe)
    {
        if (fn->val->props.find("run") == fn->val->props.end() || fn->val->props["run"]->type != TypeKind::Function)
        {
            std::cerr << TypeError("Probe has no 'run' method or it is not of type function", call->calee->token, m_context);
            exit(1);
        }

        TypePtr run = fn->val->props["run"];

        for (size_t i = 0; i < call->args.size(); i++)
        {
            TypePtr type = check(call->args[i], scope);

            if (
                run->val->params.size() <= i || type->type == TypeKind::Any || getType(run->val->params[i]->type, scope)->type == TypeKind::Any
            ) continue;

            if (!compare(getType(run->val->params[i]->type, scope), type, scope))
            {
                std::cerr
                    << TypeError("Function parameter " + std::to_string(i + 1) + " expects " + getType(run->val->params[i]->type, scope)->name + ", but got " + type->name + "\n", call->args[i]->token, m_context);
                exit(1);
            }
        }

        return std::make_shared<Type>(TypeKind::Any, "any");
    } else
    {
        std::cerr << TypeError("Only function and probes can be called, but got " + fn->name, call->calee->token, m_context);
        exit(1);
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
            std::cerr << TypeError("Object does not have property " + ident->symbol, ident->token, m_context);
            exit(1);
        }
    }

    if (obj->type == TypeKind::Module)
    {
        std::cerr << TypeError("Object does not have that property", expr->property->token, m_context);
        exit(1);
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
        scope->declareVar(param->identifier, param->staticType ? getType(param->type, env) : std::make_shared<Type>(TypeKind::Any, "any"));
    }

    for (Stmt* stmt : fn->body)
    {
        check(stmt, scope);
    }

    return std::make_shared<Type>(TypeKind::Function, "arrow function", std::make_shared<TypeVal>(fn->params));
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
            return env->declareVar(stmt->customIdent ? stmt->ident : stmt->name, lib);
        }

        TypeEnvPtr mockenv = std::make_shared<TypeEnv>();
        mockenv->declareVar(stmt->name, lib);
        TypePtr member = checkMemberExpr(static_cast<MemberExprType*>(stmt->module), mockenv);

        return env->declareVar(stmt->customIdent ? stmt->ident : static_cast<MemberExprType*>(stmt->module)->lastProp, member);
    } else
    {
        if (ctx->modules.find(stmt->name) == ctx->modules.end())
        {
            std::cerr << ManualError("Module " + stmt->name + " not found", "ImportError");
            exit(1);
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
            return env->declareVar(stmt->customIdent ? stmt->ident : stmt->name, std::make_shared<Type>(TypeKind::Module, "module", std::make_shared<TypeVal>(exports)));
        }

        TypeEnvPtr mockenv = std::make_shared<TypeEnv>();
        mockenv->declareVar(stmt->name, std::make_shared<Type>(TypeKind::Module, "module", std::make_shared<TypeVal>(exports)));
        TypePtr member = checkMemberExpr(static_cast<MemberExprType*>(stmt->module), mockenv);

        return env->declareVar(stmt->customIdent ? stmt->ident : static_cast<MemberExprType*>(stmt->module)->lastProp, member);
    }

    return std::make_shared<Type>(TypeKind::Any, "module");
}

TypePtr TC::checkNewExpr(NewExprType* expr, TypeEnvPtr env)
{
    TypePtr cls = check(expr->constructor, env);

    if (cls->type == TypeKind::Any) return cls;

    if (cls->type != TypeKind::Class)
    {
        std::cerr << TypeError("Expected constructor to be of type class, got " + cls->name, expr->constructor->token, m_context);
        exit(1);
    }

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
                        std::cerr << TypeError("Only identifiers can be exported in assignment exporting", static_cast<AssignmentExprType*>(exportstmt->exporting)->assigne->token, m_context);
                        exit(1);
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
                    std::cerr << TypeError("Unknown export type", exportstmt->exporting->token, m_context);
                    exit(1);
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

TypePtr TC::getType(TypePtr tp)
{
    if (!tp) return std::make_shared<Type>(TypeKind::Any, "any");

    TypePtr type = std::make_shared<Type>(tp);

    if (type->type == TypeKind::Class)
    {
        type->type = TypeKind::Module;
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

        return (left->type == right->type);
    }
}