#pragma once
#include <unordered_set>
#include <cmath>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include "runtime/values.hpp"
#include "frontend/ast.hpp"
#include "env.hpp"
#include "context.hpp"
#include "frontend/parser.hpp"
#include "errors.hpp"

namespace fs = std::filesystem;

extern std::unordered_map<std::string, std::pair<Probescript::Values::Val, Probescript::Typechecker::TypePtr>> g_stdlib; // Defined in stc/standard_lib/stdlib.cpp

namespace Probescript::Interpreter
{

Values::Val evalTryStmt(std::shared_ptr<AST::TryStmtType> stmt, EnvPtr env);
Values::Val evalThrowStmt(std::shared_ptr<AST::ThrowStmtType> stmt, EnvPtr env);
Values::Val evalArray(std::shared_ptr<AST::ArrayLiteralType> expr, EnvPtr env);
Values::Val evalArrowFunction(std::shared_ptr<AST::ArrowFunctionType> fn, EnvPtr env);
Values::Val evalAssignment(std::shared_ptr<AST::AssignmentExprType> assignment, EnvPtr env);
Values::Val evalBinExpr(std::shared_ptr<AST::BinaryExprType> binop, EnvPtr env);
Values::Val evalBody(std::vector<std::shared_ptr<AST::Stmt>> body, EnvPtr env, bool isLoop = false);
Values::Val evalBooleanBinExpr(std::shared_ptr<AST::BinaryExprType> binop, EnvPtr env);
Values::Val evalCall(std::shared_ptr<AST::CallExprType> call, EnvPtr env);
Values::Val evalCallWithFnVal(Values::Val fn, std::vector<Values::Val> args, EnvPtr env);
Values::Val evalClassDefinition(std::shared_ptr<AST::ClassDefinitionType> def, EnvPtr env);
Values::Val evalFunctionDeclaration(std::shared_ptr<AST::FunctionDeclarationType> declaration, EnvPtr env, bool onlyValue = false);
Values::Val evalForStmt(std::shared_ptr<AST::ForStmtType> forstmt, EnvPtr env);
Values::Val evalIdent(std::shared_ptr<AST::IdentifierType> ident, EnvPtr env);
Values::Val evalIfStmt(std::shared_ptr<AST::IfStmtType> stmt, EnvPtr baseEnv);
Values::Val evalImportStmt(std::shared_ptr<AST::ImportStmtType> importstmt, EnvPtr envptr, std::shared_ptr<Context> config);
Values::Val evalMemberAssignment(std::shared_ptr<AST::MemberAssignmentType> expr, EnvPtr env);
Values::Val evalMemberExpr(std::shared_ptr<AST::MemberExprType> expr, EnvPtr env);
Values::Val evalNewExpr(std::shared_ptr<AST::NewExprType> newexpr, EnvPtr env);
Values::Val evalObject(std::shared_ptr<AST::MapLiteralType> obj, EnvPtr env);
Values::Val evalProbeDeclaration(std::shared_ptr<AST::ProbeDeclarationType> probe, EnvPtr env);
Values::Val evalProgram(std::shared_ptr<AST::ProgramType> program, EnvPtr env, std::shared_ptr<Context> config);
Values::Val evalProbeCall(Values::Val val, EnvPtr declarationEnv, std::vector<Values::Val> args = {});
Values::Val evalVarDeclaration(std::shared_ptr<AST::VarDeclarationType> var, EnvPtr env, bool constant = false);
Values::Val evalWhileStmt(std::shared_ptr<AST::WhileStmtType> stmt, EnvPtr env);
Values::Val evalUnaryPrefix(std::shared_ptr<AST::UnaryPrefixType> expr, EnvPtr env);
Values::Val evalUnaryPostfix(std::shared_ptr<AST::UnaryPostFixType> expr, EnvPtr env);
Values::Val evalTernaryExpr(std::shared_ptr<AST::TernaryExprType> expr, EnvPtr env);
Values::Val evalTemplateCall(std::shared_ptr<AST::TemplateCallType> call, EnvPtr env);
Values::Val evalAwaitExpr(std::shared_ptr<AST::AwaitExprType> expr, EnvPtr env);

void inheritClass(std::shared_ptr<Values::ClassVal> cls, EnvPtr env, std::shared_ptr<Values::ObjectVal> thisObj, std::vector<Values::Val> args);
void inheritProbe(std::shared_ptr<Values::ProbeValue> prb, EnvPtr env);

Values::Val eval(std::shared_ptr<AST::Stmt> astNode, EnvPtr env, std::shared_ptr<Context> config = std::make_shared<Context>());

} // namespace Probescript::Interpreter