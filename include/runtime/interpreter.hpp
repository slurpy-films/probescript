#pragma once
#include "values.hpp"
#include "../ast.hpp"
#include <cmath>
#include <string>
#include <memory>
#include "env.hpp"
#include "config.hpp"

Val eval(Stmt* astNode, Env* env, Config::Config* config = new Config::Config());

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
#include "eval/memberassignment.hpp"
#include "eval/array.hpp"
#include "eval/whilestmt.hpp"
#include "eval/forstmt.hpp"
#include "eval/classdeclaration.hpp"
#include "eval/newexpr.hpp"
#include "eval/arrowfunction.hpp"

Val eval(Stmt* astNode, Env* env, Config::Config* config) {
    switch (astNode->kind) {
        case NodeType::NumericLiteral: {
            NumericLiteralType* num = static_cast<NumericLiteralType*>(astNode);
            return std::make_shared<NumberVal>(num->value());
        }

        case NodeType::StringLiteral: {
            StringLiteralType* str = static_cast<StringLiteralType*>(astNode);
            return std::make_shared<StringVal>(str->value());
        }

        case NodeType::UndefinedLiteral: {
            return std::make_shared<UndefinedVal>();
        }

        case NodeType::ReturnStmt: {
            Val value = eval(static_cast<ReturnStmtType*>(astNode)->stmt, env);
            return std::make_shared<ReturnSignal>(value);
        }
        
        case NodeType::ClassDefinition:
            return evalClassDefinition(static_cast<ClassDefinitionType*>(astNode), env);

        case NodeType::ProbeDeclaration:
            return evalProbeDeclaration(static_cast<ProbeDeclarationType*>(astNode), env);
        case NodeType::NewExpr:
            return evalNewExpr(static_cast<NewExprType*>(astNode), env);

        case NodeType::BinaryExpr:
            return evalBinExpr(static_cast<BinaryExprType*>(astNode), env);
        case NodeType::WhileStmt:
            return evalWhileStmt(static_cast<WhileStmtType*>(astNode), env);

        case NodeType::Program:
            return evalProgram(static_cast<ProgramType*>(astNode), env, config);

        case NodeType::NullLiteral:
            return std::make_shared<NullVal>();

        case NodeType::Identifier:
            return evalIdent(static_cast<IdentifierType*>(astNode), env);

        case NodeType::ObjectLiteral:
            return evalObject(static_cast<ObjectLiteralType*>(astNode), env);

        case NodeType::ArrayLiteral:
            return evalArray(static_cast<ArrayLiteralType*>(astNode), env);

        case NodeType::CallExpr:
            return evalCall(static_cast<CallExprType*>(astNode), env);

        case NodeType::VarDeclaration:
            return evalVarDeclaration(static_cast<VarDeclarationType*>(astNode), env);

        case NodeType::IfStmt:
            return evalIfStmt(static_cast<IfStmtType*>(astNode), env);

        case NodeType::FunctionDeclaration:
            return evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(astNode), env);

        case NodeType::AssignmentExpr:
            return evalAssignment(static_cast<AssignmentExprType*>(astNode), env);

        case NodeType::MemberExpr:
            return evalMemberExpr(static_cast<MemberExprType*>(astNode), env);

        case NodeType::MemberAssignment:
            return evalMemberAssignment(static_cast<MemberAssignmentType*>(astNode), env);

        case NodeType::ForStmt:
            return evalForStmt(static_cast<ForStmtType*>(astNode), env);

        case NodeType::UnaryPostFix:
            return evalUnaryPostfix(static_cast<UnaryPostFixType*>(astNode), env);
        
        case NodeType::UnaryPrefix:
            return evalUnaryPrefix(static_cast<UnaryPrefixType*>(astNode), env);

        case NodeType::ArrowFunction:
            return evalArrowFunction(static_cast<ArrowFunctionType*>(astNode), env);

        default:
            std::cout << "Unexpected AST-node kind found: ";
            std::cout << astNode->kind << std::endl;
            exit(1);
    }
}