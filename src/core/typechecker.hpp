#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include "frontend/ast.hpp"
#include "errors.hpp"
#include "context.hpp"
#include "frontend/parser.hpp"
#include "types.hpp"

extern std::unordered_map<std::string, std::pair<Val, TypePtr>> g_stdlib;
extern std::unordered_map<std::string, std::pair<Val, TypePtr>> g_globals;

namespace fs = std::filesystem;

inline std::unordered_set<std::string> boolOps = { "&&", "||", ">=", "<=", "<", ">", "!=", "==" };

class TC
{
public:
    void checkProgram(ProgramType* program, TypeEnvPtr env, std::shared_ptr<Context> ctx = std::make_shared<Context>());
private:
    std::shared_ptr<Context> m_context;
    TypePtr m_currentret;

    TypePtr check(Stmt* node, TypeEnvPtr env, std::shared_ptr<Context> ctx = std::make_shared<Context>());

    TypePtr checkVarDecl(VarDeclarationType* decl, TypeEnvPtr env);
    TypePtr checkAssign(AssignmentExprType* assign, TypeEnvPtr env);
    TypePtr checkIdent(IdentifierType* ident, TypeEnvPtr env);
    TypePtr checkProbe(ProbeDeclarationType* prb, TypeEnvPtr env);
    TypePtr checkFunction(FunctionDeclarationType* fn, TypeEnvPtr env);
    TypePtr checkCall(CallExprType* call, TypeEnvPtr env);
    TypePtr checkObjectExpr(MapLiteralType* obj, TypeEnvPtr env);
    TypePtr checkMemberExpr(MemberExprType* expr, TypeEnvPtr env);
    TypePtr checkImportStmt(ImportStmtType* stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx);
    TypePtr checkExportStmt(Stmt* stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx);
    TypePtr checkBinExpr(BinaryExprType* expr, TypeEnvPtr env);
    TypePtr checkClassDeclaration(ClassDefinitionType* cls, TypeEnvPtr env);
    TypePtr checkNewExpr(NewExprType* expr, TypeEnvPtr env);
    TypePtr checkIfStmt(IfStmtType* stmt, TypeEnvPtr env);
    TypePtr checkForStmt(ForStmtType* stmt, TypeEnvPtr env);
    TypePtr checkMemberAssign(MemberAssignmentType* assign, TypeEnvPtr env);
    TypePtr checkArrayLiteral(ArrayLiteralType* array, TypeEnvPtr env);
    TypePtr checkArrowFunction(ArrowFunctionType* fn, TypeEnvPtr env);
    TypePtr checkTernaryExpr(TernaryExprType* expr, TypeEnvPtr env);
    TypePtr checkReturnStmt(ReturnStmtType* stmt, TypeEnvPtr env);
    TypePtr checkTemplateCall(TemplateCallType* call, TypeEnvPtr env);
    TypePtr checkCastExpr(CastExprType* expr, TypeEnvPtr env);
    TypePtr checkUnaryPrefix(UnaryPrefixType* expr, TypeEnvPtr env);

    void checkProbeInheritance(TypePtr prb, TypeEnvPtr env);
    void checkClassInheritance(TypePtr cls, TypeEnvPtr env, TypePtr thisobj);

    std::unordered_map<std::string, TypePtr> getExports(ProgramType* program, std::shared_ptr<Context> ctx);

    TypePtr getType(Expr* name, TypeEnvPtr env);
    bool compare(TypePtr left, TypePtr right, TypeEnvPtr env);
};