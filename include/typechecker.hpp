#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include "ast.hpp"
#include "errors.hpp"
#include "config.hpp"
#include "parser.hpp"

namespace fs = std::filesystem;

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

struct Type;

struct TypeVal
{
    std::vector<VarDeclarationType*> params;
    std::unordered_map<std::string, Type> props;

    TypeVal(std::vector<VarDeclarationType*> params)
        : params(params) {}
    TypeVal(std::unordered_map<std::string, Type> props)
        : props(props) {}
    TypeVal() {}
};

struct Type
{
    TypeKind type;
    std::string name;
    TypeVal val;

    Type(TypeKind type, std::string name)
        : type(type), name(name) {}

    Type(TypeKind type, std::string name, TypeVal value)
        : type(type), name(name), val(value) {}
    
    Type()
        : type(TypeKind::Any), name("any") {}
};

class TypeEnv
{
public:
    TypeEnv(std::shared_ptr<TypeEnv> parent = nullptr);

    Type declareVar(std::string name, Type type);
    Type lookUp(std::string name);
private:
    std::unordered_map<std::string, Type> m_variables;
    std::shared_ptr<TypeEnv> m_parent;
};

using TypeEnvPtr = std::shared_ptr<TypeEnv>;

class TC
{
public:
    void checkProgram(ProgramType* program, TypeEnvPtr env, Context* ctx = new Context());
private:
    Type check(Stmt* node, TypeEnvPtr env, Context* ctx = new Context());

    Type checkVarDecl(VarDeclarationType* decl, TypeEnvPtr env);
    Type checkAssign(AssignmentExprType* assign, TypeEnvPtr env);
    Type checkIdent(IdentifierType* ident, TypeEnvPtr env);
    Type checkProbe(ProbeDeclarationType* prb, TypeEnvPtr env);
    Type checkFunction(FunctionDeclarationType* fn, TypeEnvPtr env);
    Type checkCall(CallExprType* call, TypeEnvPtr env);
    Type checkObjectExpr(MapLiteralType* obj, TypeEnvPtr env);
    Type checkMemberExpr(MemberExprType* expr, TypeEnvPtr env);
    Type checkIimportStmt(ImportStmtType* stmt, TypeEnvPtr env, Context* ctx);
    Type checkExportStmt(Stmt* stmt, TypeEnvPtr env, Context* ctx);

    Type getType(Expr* name);
    bool compare(Type left, Type right);
};