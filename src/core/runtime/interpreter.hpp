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

namespace fs = std::filesystem;

extern std::unordered_map<std::string, std::pair<Val, TypePtr>> g_stdlib; // Defined in stc/standard_lib/stdlib.cpp

Val evalTryStmt(TryStmtType* stmt, EnvPtr env);
Val evalThrowStmt(ThrowStmtType* stmt, EnvPtr env);
Val evalArray(ArrayLiteralType* expr, EnvPtr env);
Val evalArrowFunction(ArrowFunctionType* fn, EnvPtr env);
Val evalAssignment(AssignmentExprType* assignment, EnvPtr env);
Val evalBinExpr(BinaryExprType* binop, EnvPtr env);
Val evalBody(std::vector<Stmt*> body, EnvPtr env, bool isLoop = false);
Val evalBooleanBinExpr(BinaryExprType* binop, EnvPtr env);
Val evalCall(CallExprType* call, EnvPtr env);
Val evalCallWithFnVal(Val fn, std::vector<Val> args, EnvPtr env);
Val evalClassDefinition(ClassDefinitionType* def, EnvPtr env);
Val evalFunctionDeclaration(FunctionDeclarationType* declaration, EnvPtr env, bool onlyValue = false);
Val evalForStmt(ForStmtType* forstmt, EnvPtr env);
Val evalIdent(IdentifierType* ident, EnvPtr env);
Val evalIfStmt(IfStmtType* stmt, EnvPtr baseEnv);
Val evalImportStmt(ImportStmtType* importstmt, EnvPtr envptr, std::shared_ptr<Context> config);
Val evalMemberAssignment(MemberAssignmentType* expr, EnvPtr env);
Val evalMemberExpr(MemberExprType* expr, EnvPtr env);
Val evalNewExpr(NewExprType* newexpr, EnvPtr env);
std::shared_ptr<NumberVal> evalNumericBinExpr(std::shared_ptr<NumberVal> lhs, std::shared_ptr<NumberVal> rhs, std::string op);
Val evalObject(MapLiteralType* obj, EnvPtr env);
Val evalProbeDeclaration(ProbeDeclarationType* probe, EnvPtr env);
Val evalProgram(ProgramType* program, EnvPtr env, std::shared_ptr<Context> config);
Val evalProbeCall(std::string probeName, EnvPtr declarationEnv, std::vector<Val> args = {});
std::shared_ptr<StringVal> evalStringericBinExpr(std::shared_ptr<StringVal> lhs, std::shared_ptr<StringVal> rhs, std::string op);
Val evalVarDeclaration(VarDeclarationType* var, EnvPtr env, bool constant = false);
Val evalWhileStmt(WhileStmtType* stmt, EnvPtr env);
Val evalUnaryPrefix(UnaryPrefixType* expr, EnvPtr env);
Val evalUnaryPostfix(UnaryPostFixType* expr, EnvPtr env);
Val evalTernaryExpr(TernaryExprType* expr, EnvPtr env);

void inheritClass(std::shared_ptr<ClassVal> cls, EnvPtr env, std::shared_ptr<ObjectVal> thisObj, std::vector<Val> args);
void inheritProbe(std::shared_ptr<ProbeValue> prb, EnvPtr env);

Val eval(Stmt* astNode, EnvPtr env, std::shared_ptr<Context> config = std::make_shared<Context>());