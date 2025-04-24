#pragma once
#include "values.hpp"
#include "../ast.hpp"
#include <cmath>
#include <string>
#include "env.hpp"

RuntimeVal* eval(Stmt* astNode, Env* env, string ProbeName = "Main");

#include "eval/program.hpp"
#include "eval/probedeclaration.hpp"
#include "eval/assignment.hpp"
#include "eval/fndeclaration.hpp"
#include "eval/numbinexpr.hpp"
#include "eval/stringbinexpr.hpp"
#include "eval/ifstmt.hpp"
#include "eval/memberexpr.hpp"
#include "eval/binexpr.hpp"
#include "eval/identifier.hpp"
#include "eval/object.hpp"
#include "eval/call.hpp"
#include "eval/vardeclaration.hpp"
#include "eval/boolbinop.hpp"
#include "eval/runprobe.hpp"

RuntimeVal* eval(Stmt* astNode, Env* env, string ProbeName) {
    switch (astNode->kind) {
        case NodeType::NumericLiteral: {
            NumericLiteralType* num = static_cast<NumericLiteralType*>(astNode);
            return new NumberVal(num->value());
        }

        case NodeType::StringLiteral: {
            StringLiteralType* str = static_cast<StringLiteralType*>(astNode);
            return new StringVal(str->value());
        }

        case NodeType::ProbeDeclaration:
            return evalProbeDeclaration(static_cast<ProbeDeclarationType*>(astNode), env);

        case NodeType::BinaryExpr:
            return evalBinExpr(static_cast<BinaryExprType*>(astNode), env);

        case NodeType::Program:
            return evalProgram(static_cast<ProgramType*>(astNode), env, ProbeName);

        case NodeType::NullLiteral:
            return new NullVal();

        case NodeType::Identifier:
            return evalIdent(static_cast<IdentifierType*>(astNode), env);

        case NodeType::ObjectLiteral:
            return evalObject(static_cast<ObjectLiteralType*>(astNode), env);

        case NodeType::CallExpr:
            return evalCall(static_cast<CallExprType*>(astNode), env);

        case NodeType::VarDeclaration:
            return evalVarDeclaration(static_cast<VarDecalarationType*>(astNode), env);

        case NodeType::IfStmt:
            return evalIfStmt(static_cast<IfStmtType*>(astNode), env);

        case NodeType::FunctionDeclaration:
            return evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(astNode), env);

        case NodeType::AssignmentExpr:
            return evalAssignment(static_cast<AssignmentExprType*>(astNode), env);

        case NodeType::MemberExpr:
            return evalMemberExpr(static_cast<MemberExprType*>(astNode), env);

        default:
            cout << "Unexpected AST-node kind found: ";
            cout << astNode->kind << endl;
            return new NullVal();
    }
}