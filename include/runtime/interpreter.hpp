#pragma once
#include <unordered_set>
#include <cmath>
#include <string>
#include <memory>
#include "values.hpp"
#include "ast.hpp"
#include "env.hpp"
#include "config.hpp"
#include "parser.hpp"
#include "stdlib/stdlib.hpp"
#include "stdlib/array.hpp"

Val evalTryStmt(TryStmtType* stmt, Env* env);
Val evalThrowStmt(ThrowStmtType* stmt, Env* env);
Val evalArray(ArrayLiteralType* expr, Env* env);
Val evalArrowFunction(ArrowFunctionType* fn, Env* env);
Val evalAssignment(AssignmentExprType* assignment, Env* env);
Val evalBinExpr(BinaryExprType* binop, Env* env);
Val evalBody(std::vector<Stmt*> body, Env* env, bool isLoop = false);
Val evalBooleanBinExpr(BinaryExprType* binop, Env* env);
Val evalCall(CallExprType* call, Env* env);
Val evalCallWithFnVal(Val fn, std::vector<Val> args, Env* env);
Val evalClassDefinition(ClassDefinitionType* def, Env* env);
Val evalFunctionDeclaration(FunctionDeclarationType* declaration, Env* env, bool onlyValue = false);
Val evalForStmt(ForStmtType* forstmt, Env* env);
Val evalIdent(IdentifierType* ident, Env* env);
Val evalIfStmt(IfStmtType* stmt, Env* baseEnv);
Val evalImportStmt(ImportStmtType* importstmt, Env* envptr, Config::Config* config);
Val evalMemberAssignment(MemberAssignmentType* expr, Env* env);
Val evalMemberExpr(MemberExprType* expr, Env* env);
Val evalNewExpr(NewExprType* newexpr, Env* env);
void inheritClass(std::shared_ptr<ClassVal> cls, Env* env, std::shared_ptr<ObjectVal> thisObj, std::vector<Val> args);
std::shared_ptr<NumberVal> evalNumericBinExpr(std::shared_ptr<NumberVal> lhs, std::shared_ptr<NumberVal> rhs, std::string op);
Val evalObject(ObjectLiteralType* obj, Env* env);
Val evalProbeDeclaration(ProbeDeclarationType* probe, Env* env);
Val evalProgram(ProgramType* program, Env* env, Config::Config* config);
Val evalProbeCall(std::string probeName, Env* declarationEnv, std::vector<Val> args = {});
std::shared_ptr<StringVal> evalStringericBinExpr(std::shared_ptr<StringVal> lhs, std::shared_ptr<StringVal> rhs, std::string op);
Val evalVarDeclaration(VarDeclarationType* var, Env* env, bool constant = false);
Val evalWhileStmt(WhileStmtType* stmt, Env* env);
Val evalUnaryPrefix(UnaryPrefixType* expr, Env* env);
Val evalUnaryPostfix(UnaryPostFixType* expr, Env* env);

Val eval(Stmt* astNode, Env* env, Config::Config* config = new Config::Config());