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

extern std::unordered_map<std::string, std::pair<Val, TypePtr>> g_stdlib; // Defined in stc/standard_lib/stdlib.cpp

Val evalTryStmt(std::shared_ptr<TryStmtType> stmt, EnvPtr env);
Val evalThrowStmt(std::shared_ptr<ThrowStmtType> stmt, EnvPtr env);
Val evalArray(std::shared_ptr<ArrayLiteralType> expr, EnvPtr env);
Val evalArrowFunction(std::shared_ptr<ArrowFunctionType> fn, EnvPtr env);
Val evalAssignment(std::shared_ptr<AssignmentExprType> assignment, EnvPtr env);
Val evalBinExpr(std::shared_ptr<BinaryExprType> binop, EnvPtr env);
Val evalBody(std::vector<std::shared_ptr<Stmt>> body, EnvPtr env, bool isLoop = false);
Val evalBooleanBinExpr(std::shared_ptr<BinaryExprType> binop, EnvPtr env);
Val evalCall(std::shared_ptr<CallExprType> call, EnvPtr env);
Val evalCallWithFnVal(Val fn, std::vector<Val> args, EnvPtr env);
Val evalClassDefinition(std::shared_ptr<ClassDefinitionType> def, EnvPtr env);
Val evalFunctionDeclaration(std::shared_ptr<FunctionDeclarationType> declaration, EnvPtr env, bool onlyValue = false);
Val evalForStmt(std::shared_ptr<ForStmtType> forstmt, EnvPtr env);
Val evalIdent(std::shared_ptr<IdentifierType> ident, EnvPtr env);
Val evalIfStmt(std::shared_ptr<IfStmtType> stmt, EnvPtr baseEnv);
Val evalImportStmt(std::shared_ptr<ImportStmtType> importstmt, EnvPtr envptr, std::shared_ptr<Context> config);
Val evalMemberAssignment(std::shared_ptr<MemberAssignmentType> expr, EnvPtr env);
Val evalMemberExpr(std::shared_ptr<MemberExprType> expr, EnvPtr env);
Val evalNewExpr(std::shared_ptr<NewExprType> newexpr, EnvPtr env);
Val evalObject(std::shared_ptr<MapLiteralType> obj, EnvPtr env);
Val evalProbeDeclaration(std::shared_ptr<ProbeDeclarationType> probe, EnvPtr env);
Val evalProgram(std::shared_ptr<ProgramType> program, EnvPtr env, std::shared_ptr<Context> config);
Val evalProbeCall(Val val, EnvPtr declarationEnv, std::vector<Val> args = {});
Val evalVarDeclaration(std::shared_ptr<VarDeclarationType> var, EnvPtr env, bool constant = false);
Val evalWhileStmt(std::shared_ptr<WhileStmtType> stmt, EnvPtr env);
Val evalUnaryPrefix(std::shared_ptr<UnaryPrefixType> expr, EnvPtr env);
Val evalUnaryPostfix(std::shared_ptr<UnaryPostFixType> expr, EnvPtr env);
Val evalTernaryExpr(std::shared_ptr<TernaryExprType> expr, EnvPtr env);
Val evalTemplateCall(std::shared_ptr<TemplateCallType> call, EnvPtr env);
Val evalAwaitExpr(std::shared_ptr<AwaitExprType> expr, EnvPtr env);

void inheritClass(std::shared_ptr<ClassVal> cls, EnvPtr env, std::shared_ptr<ObjectVal> thisObj, std::vector<Val> args);
void inheritProbe(std::shared_ptr<ProbeValue> prb, EnvPtr env);

Val eval(std::shared_ptr<Stmt> astNode, EnvPtr env, std::shared_ptr<Context> config = std::make_shared<Context>());