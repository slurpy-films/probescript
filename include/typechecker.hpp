#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include "ast.hpp"
#include "errors.hpp"
#include "context.hpp"
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
    Class,
    Probe,
    Module, // A module is handled like an object, but if the user tries to access a member it doesn't have, it throws the process
};

struct Type;
using TypePtr = std::shared_ptr<Type>;

struct TypeVal
{
    std::vector<VarDeclarationType*> params = {};
    std::unordered_map<std::string, TypePtr> props = {};

    TypeVal(std::vector<VarDeclarationType*> params)
        : params(params) {}
    TypeVal(std::unordered_map<std::string, TypePtr> props)
        : props(props) {}
    TypeVal() {}
};

using TypeValPtr = std::shared_ptr<TypeVal>;

struct Type
{
    TypeKind type;
    std::string name;
    TypeValPtr val = std::make_shared<TypeVal>();
    int typeID;
    std::string typeName;
    bool isInstance = false;

    Type(TypeKind type, std::string name, std::string typeName = "")
        : type(type), name(name), typeName(typeName) {}

    Type(TypeKind type, std::string name, bool isInstance)
        : type(type), name(name), isInstance(isInstance) {}

    Type(TypeKind type, std::string name, TypeValPtr value, std::string typeName = "")
        : type(type), name(name), val(value), typeName(typeName) {}
    
    Type()
        : type(TypeKind::Any), name("any") {}
};


#include "stdlib/stdlib.hpp"

class TypeEnv
{
public:
    TypeEnv(std::shared_ptr<TypeEnv> parent = nullptr);

    TypePtr declareVar(std::string name, TypePtr type);
    TypePtr lookUp(std::string name);

    std::unordered_map<std::string, TypePtr> getVars();
private:
    std::unordered_map<std::string, TypePtr> m_variables;
    std::shared_ptr<TypeEnv> m_parent;
};

using TypeEnvPtr = std::shared_ptr<TypeEnv>;

inline std::unordered_set<std::string> boolOps = { "&&", "||", ">=", "<=", "<", ">", "!=", "==" };

class TC
{
public:
    void checkProgram(ProgramType* program, TypeEnvPtr env, Context* ctx = new Context());
private:
    int m_typeId = 0;

    TypePtr check(Stmt* node, TypeEnvPtr env, Context* ctx = new Context());

    TypePtr checkVarDecl(VarDeclarationType* decl, TypeEnvPtr env);
    TypePtr checkAssign(AssignmentExprType* assign, TypeEnvPtr env);
    TypePtr checkIdent(IdentifierType* ident, TypeEnvPtr env);
    TypePtr checkProbe(ProbeDeclarationType* prb, TypeEnvPtr env);
    TypePtr checkFunction(FunctionDeclarationType* fn, TypeEnvPtr env);
    TypePtr checkCall(CallExprType* call, TypeEnvPtr env);
    TypePtr checkObjectExpr(MapLiteralType* obj, TypeEnvPtr env);
    TypePtr checkMemberExpr(MemberExprType* expr, TypeEnvPtr env);
    TypePtr checkImportStmt(ImportStmtType* stmt, TypeEnvPtr env, Context* ctx);
    TypePtr checkExportStmt(Stmt* stmt, TypeEnvPtr env, Context* ctx);
    TypePtr checkBinExpr(BinaryExprType* expr, TypeEnvPtr env);
    TypePtr checkClassDeclaration(ClassDefinitionType* cls, TypeEnvPtr env);
    TypePtr checkNewExpr(NewExprType* expr, TypeEnvPtr env);
    TypePtr checkIfStmt(IfStmtType* stmt, TypeEnvPtr env);
    TypePtr checkForStmt(ForStmtType* stmt, TypeEnvPtr env);
    TypePtr checkMemberAssign(MemberAssignmentType* assign, TypeEnvPtr env);

    std::unordered_map<std::string, TypePtr> getExports(ProgramType* program, Context* ctx);

    TypePtr getType(Expr* name, TypeEnvPtr env);
    bool compare(TypePtr left, TypePtr right, TypeEnvPtr env);
};