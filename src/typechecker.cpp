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
        },
        {
            "undefined",
            Type(TypeKind::Undefined, "undefined")
        },
        {
            "true",
            Type(TypeKind::Bool, "bool")
        },
        {
            "false",
            Type(TypeKind::Bool, "bool")
        },
    };
}

void TypeEnv::declareVar(std::string name, Type type)
{
    if (m_variables.find(name) != m_variables.end())
    {
        std::cout << TypeError("Variable " + name + " is already defined");
        exit(1);
    }

    m_variables[name] = type;
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

void TC::checkProgram(ProgramType* program, TypeEnvPtr env)
{
    for (Stmt* stmt : program->body)
    {
        check(stmt, env);
    }
}

Type TC::check(Stmt* node, TypeEnvPtr env)
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
        default:
            return Type(TypeKind::Any, "any");
    }
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

    return Type(TypeKind::Any, "any");
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