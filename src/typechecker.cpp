#include "typechecker.hpp"

TypeEnv::TypeEnv(std::shared_ptr<TypeEnv> parent)
{
    if (parent) m_parent = parent;

    std::unordered_map<std::string, Type> constants = 
    {
        {
            "process",
            Type(TypeKind::Object, "object")
        },
        {
            "num",
            Type(TypeKind::Function, "native function")
        },
        {
            "str",
            Type(TypeKind::Function, "native function")
        },
        {
            "console",
            Type(TypeKind::Object, "Object")
        }
    };

    m_variables = constants;
}

Type TypeEnv::declareVar(std::string name, Type type)
{
    if (m_variables.find(name) != m_variables.end())
    {
        std::cout << TypeError("Variable " + name + " is already defined");
        exit(1);
    }

    m_variables[name] = type;

    return type;
}

Type TypeEnv::lookUp(std::string name)
{
    if (m_variables.find(name) != m_variables.end())
        return m_variables[name];
    else if (m_parent)
        return m_parent->lookUp(name);
    else
    {
        std::cerr << TypeError("Variable " + name + " is not defined");
        exit(1);
    }
}

void TC::checkProgram(ProgramType* program, TypeEnvPtr env, Context* ctx)
{
    for (Stmt* stmt : program->body)
    {
        check(stmt, env, ctx);
    }
}

Type TC::check(Stmt* node, TypeEnvPtr env, Context* ctx)
{
    switch (node->kind)
    {
        case NodeType::NumericLiteral:
            return Type(TypeKind::Number, "number");
        case NodeType::StringLiteral:
            return Type(TypeKind::String, "string");
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
            return checkIimportStmt(static_cast<ImportStmtType*>(node), env, ctx);
        case NodeType::ExportStmt:
            return checkExportStmt(static_cast<ExportStmtType*>(node)->exporting, env, ctx);
        default:
            return Type(TypeKind::Any, "any");
    }
}

Type TC::checkExportStmt(Stmt* stmt, TypeEnvPtr env, Context* ctx)
{
    if (stmt->kind == NodeType::AssignmentExpr)
    {
        AssignmentExprType* assign = static_cast<AssignmentExprType*>(stmt);

        if (assign->assigne->kind != NodeType::Identifier)
        {
            std::cerr << TypeError("Identifier exporting can only be used on identifiers");
            exit(1);
        }

        return check(new VarDeclarationType(assign->value, static_cast<IdentifierType*>(assign->assigne)->symbol), env, ctx);
    }

    return check(stmt, env, ctx);
}

Type TC::checkVarDecl(VarDeclarationType* decl, TypeEnvPtr env)
{
    if (decl->staticType && decl->value->kind != NodeType::UndefinedLiteral)
    {
        Type vartype = getType(decl->type);
        Type assigntype = check(decl->value, env);
        if (!compare(vartype, assigntype))
        {
            std::cerr << TypeError("Cannot convert " + assigntype.name + " to " + vartype.name);
            exit(1);
        }
    }

    check(decl->value, env);
    env->declareVar(decl->identifier, decl->staticType ? getType(decl->type) : Type(TypeKind::Any, "any"));

    return Type(TypeKind::Any, "any");
}

Type TC::checkIdent(IdentifierType* ident, TypeEnvPtr env)
{
    return env->lookUp(ident->symbol);
}

Type TC::checkAssign(AssignmentExprType* assign, TypeEnvPtr env)
{
    Type assigne = check(assign->assigne, env);
    Type value = check(assign->value, env);

    if (!compare(assigne, value))
    {
        std::cerr << TypeError("Cannot convert " + value.name + " to " + assigne.name);
        exit(1);
    }

    return value;
}

Type TC::checkProbe(ProbeDeclarationType* prb, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    for (Stmt* stmt : prb->body)
    {
        check(stmt, scope);
    }

    return Type(TypeKind::Any, "any");
}

Type TC::checkFunction(FunctionDeclarationType* fn, TypeEnvPtr env)
{
    TypeEnvPtr scope = std::make_shared<TypeEnv>(env);

    
    for (VarDeclarationType* param : fn->parameters)
    {
        scope->declareVar(param->identifier, param->staticType ? getType(param->type) : Type(TypeKind::Any, "any"));
    }

    for (Stmt* stmt : fn->body)
    {
        check(stmt, scope);
    }

    env->declareVar(fn->name, Type(TypeKind::Function, "function", TypeVal(fn->parameters)));
    return Type(TypeKind::Any, "any");
}

Type TC::checkCall(CallExprType* call, TypeEnvPtr env)
{
    if (call->calee->kind == NodeType::Identifier)
    {
        Type fn = env->lookUp(static_cast<IdentifierType*>(call->calee)->symbol);

        if (fn.type != TypeKind::Function && fn.type != TypeKind::Any)
        {
            std::cerr << TypeError("Cannot call value that is not a function: " + static_cast<IdentifierType*>(call->calee)->symbol);
            exit(1);
        }

        for (size_t i = 0; i < call->args.size(); i++)
        {
            Type type = check(call->args[i], env);

            if (
                fn.val.params.size() < i || type.type == TypeKind::Any || getType(fn.val.params[i]->type).type == TypeKind::Any
            ) continue;

            if (!compare(type, getType(fn.val.params[i]->type)))
            {
                std::cerr
                    << TypeError("Function " + static_cast<IdentifierType*>(call->calee)->symbol + " parameter " + std::to_string(i) + " expects " + getType(fn.val.params[i]->type).name + ". " + type.name + " given");
                exit(1);
            }
        }
    }

    return Type(TypeKind::Any, "any");
}

Type TC::checkObjectExpr(MapLiteralType* obj, TypeEnvPtr env)
{
    return Type(TypeKind::Object, "map");
}

Type TC::checkMemberExpr(MemberExprType* expr, TypeEnvPtr env)
{
    Type obj = check(expr->object, env);

    return Type(TypeKind::Any, "any");
}

Type TC::checkIimportStmt(ImportStmtType* stmt, TypeEnvPtr env, Context* ctx)
{
    if (stmt->name == "http" || stmt->name == "random" || stmt->name == "json" || stmt->name == "fs")
    {        
        if (!stmt->hasMember || stmt->customIdent)
        {
            return env->declareVar(stmt->customIdent ? stmt->ident : stmt->name, Type(TypeKind::Any, "native module"));
        }

        return env->declareVar(static_cast<MemberExprType*>(stmt->module)->lastProp, Type(TypeKind::Any, "native module"));
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
        
        ProgramType* program = parser.produceAST(file);
        TC tc;
        tc.checkProgram(program, std::make_shared<TypeEnv>());

        if (!stmt->hasMember || stmt->customIdent)
        {
            return env->declareVar(stmt->customIdent ? stmt->ident : stmt->name, Type(TypeKind::Any, "module"));
        }


        return env->declareVar(static_cast<MemberExprType*>(stmt->module)->lastProp, Type(TypeKind::Any, "module"));
    }

    return Type(TypeKind::Any, "module");
}

Type TC::getType(Expr* name)
{
    if (!name) return Type(TypeKind::Any, "any");

    if (name->kind == NodeType::Identifier)
    {
        IdentifierType* ident = static_cast<IdentifierType*>(name);

        if (ident->symbol == "str")
            return Type(TypeKind::String, "string");
        else if (ident->symbol == "num")
            return Type(TypeKind::Number, "number");
        else if (ident->symbol == "bool")
            return Type(TypeKind::Bool, "bool");
        else if (ident->symbol == "map")
            return Type(TypeKind::Object, "map");
        else
            return Type();
    } else return Type();
}

bool TC::compare(Type left, Type right)
{
    if (left.type == TypeKind::Any || right.type == TypeKind::Any)
        return true;
    else
        return left.type == right.type;
}