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

extern std::unordered_map<std::string, std::pair<Probescript::Values::Val, Probescript::Typechecker::TypePtr>> g_stdlib;
extern std::unordered_map<std::string, std::pair<Probescript::Values::Val, Probescript::Typechecker::TypePtr>> g_globals;

namespace fs = std::filesystem;

namespace Probescript::Typechecker
{

inline std::unordered_set<std::string> boolOps = { "&&", "||", ">=", "<=", "<", ">", "!=", "==" };

class TC
{
public:
    void checkProgram(std::shared_ptr<AST::ProgramType> program, TypeEnvPtr env, std::shared_ptr<Context> ctx = std::make_shared<Context>());
private:
    std::shared_ptr<Context> m_context;
    TypePtr m_currentret;

    TypePtr check(std::shared_ptr<AST::Stmt> node, TypeEnvPtr env, std::shared_ptr<Context> ctx = std::make_shared<Context>());

    TypePtr checkVarDecl(std::shared_ptr<AST::VarDeclarationType> decl, TypeEnvPtr env);
    TypePtr checkAssign(std::shared_ptr<AST::AssignmentExprType> assign, TypeEnvPtr env);
    TypePtr checkIdent(std::shared_ptr<AST::IdentifierType> ident, TypeEnvPtr env);
    TypePtr checkProbe(std::shared_ptr<AST::ProbeDeclarationType> prb, TypeEnvPtr env);
    TypePtr checkFunction(std::shared_ptr<AST::FunctionDeclarationType> fn, TypeEnvPtr env, bool templateProcessed = false);
    TypePtr checkCall(std::shared_ptr<AST::CallExprType> call, TypeEnvPtr env);
    TypePtr checkObjectExpr(std::shared_ptr<AST::MapLiteralType> obj, TypeEnvPtr env);
    TypePtr checkMemberExpr(std::shared_ptr<AST::MemberExprType> expr, TypeEnvPtr env);
    TypePtr checkImportStmt(std::shared_ptr<AST::ImportStmtType> stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx);
    TypePtr checkExportStmt(std::shared_ptr<AST::Stmt> stmt, TypeEnvPtr env, std::shared_ptr<Context> ctx);
    TypePtr checkBinExpr(std::shared_ptr<AST::BinaryExprType> expr, TypeEnvPtr env);
    TypePtr checkClassDeclaration(std::shared_ptr<AST::ClassDefinitionType> cls, TypeEnvPtr env);
    TypePtr checkNewExpr(std::shared_ptr<AST::NewExprType> expr, TypeEnvPtr env);
    TypePtr checkIfStmt(std::shared_ptr<AST::IfStmtType> stmt, TypeEnvPtr env);
    TypePtr checkForStmt(std::shared_ptr<AST::ForStmtType> stmt, TypeEnvPtr env);
    TypePtr checkMemberAssign(std::shared_ptr<AST::MemberAssignmentType> assign, TypeEnvPtr env);
    TypePtr checkArrayLiteral(std::shared_ptr<AST::ArrayLiteralType> array, TypeEnvPtr env);
    TypePtr checkArrowFunction(std::shared_ptr<AST::ArrowFunctionType> fn, TypeEnvPtr env);
    TypePtr checkTernaryExpr(std::shared_ptr<AST::TernaryExprType> expr, TypeEnvPtr env);
    TypePtr checkReturnStmt(std::shared_ptr<AST::ReturnStmtType> stmt, TypeEnvPtr env);
    TypePtr checkTemplateCall(std::shared_ptr<AST::TemplateCallType> call, TypeEnvPtr env);
    TypePtr checkCastExpr(std::shared_ptr<AST::CastExprType> expr, TypeEnvPtr env);
    TypePtr checkUnaryPrefix(std::shared_ptr<AST::UnaryPrefixType> expr, TypeEnvPtr env);

    void checkProbeInheritance(TypePtr prb, TypeEnvPtr env);
    void checkClassInheritance(TypePtr cls, TypeEnvPtr env, TypePtr thisobj);

    std::unordered_map<std::string, TypePtr> getExports(std::shared_ptr<AST::ProgramType> program, std::shared_ptr<Context> ctx);

    TypePtr getType(std::shared_ptr<AST::Expr> name, TypeEnvPtr env);
    bool compare(TypePtr left, TypePtr right, TypeEnvPtr env);
};

} // namespace Probescript::Typechecker