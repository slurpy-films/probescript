#pragma once
#include "ast.hpp"
#include <unordered_map>
#include <string>
#include "errors.hpp"


enum class TypeKind
{
    Number,
    String,
    Any,
    Function,
    Custom,
    Object,
    Undefined,
    Bool,
};

struct Type
{
    TypeKind type;
    std::string name;
    Stmt* value;

    Type(TypeKind type, std::string name)
        : type(type), name(name) {}

    Type(TypeKind type, std::string name, Stmt* value)
        : type(type), name(name), value(value) {}
    
    Type()
        : type(TypeKind::Any), name("any") {}
};

class TypeEnv
{
public:
    TypeEnv(std::shared_ptr<TypeEnv> parent = nullptr);

    void declareVar(std::string name, Type type);
    Type lookUp(std::string name);
private:
    std::unordered_map<std::string, Type> m_variables;
    std::shared_ptr<TypeEnv> m_parent;
};

using TypeEnvPtr = std::shared_ptr<TypeEnv>;

class TC
{
public:
    void checkProgram(ProgramType* program, TypeEnvPtr env);
private:
    Type check(Stmt* node, TypeEnvPtr env);

    Type checkVarDecl(VarDeclarationType* decl, TypeEnvPtr env);
    Type checkAssign(AssignmentExprType* assign, TypeEnvPtr env);
    Type checkIdent(IdentifierType* ident, TypeEnvPtr env);
    Type checkProbe(ProbeDeclarationType* prb, TypeEnvPtr env);
    Type checkFunction(FunctionDeclarationType* fn, TypeEnvPtr env);

    Type getType(Expr* name);
    bool compare(Type left, Type right);
};