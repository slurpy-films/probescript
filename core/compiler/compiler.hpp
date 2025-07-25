#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "frontend/ast.hpp"

#include "vm/builder.hpp"

#include "core/errors.hpp"

namespace Probescript
{

class Compiler
{
public:
    Compiler(std::shared_ptr<AST::ProgramType> program)
        : m_program(program), builder(std::make_shared<VM::ByteCodeBuilder>()) {}

    void compile();

    std::vector<std::shared_ptr<VM::Instruction>> getInstructions();
    std::vector<VM::ValuePtr> getConstants();
private:
    std::shared_ptr<VM::ByteCodeBuilder> builder;
    std::shared_ptr<AST::ProgramType> m_program;
        
    void gen(std::shared_ptr<AST::Stmt> node);
    
    // Statement generator methods
    void genFunction(std::shared_ptr<AST::FunctionDeclarationType> fn);
    void genVarDecl(std::shared_ptr<AST::VarDeclarationType> decl);
    void genIf(std::shared_ptr<AST::IfStmtType> stmt);
    void genWhile(std::shared_ptr<AST::WhileStmtType> stmt);
    void genReturn(std::shared_ptr<AST::ReturnStmtType> stmt);
    void genProbe(std::shared_ptr<AST::ProbeDeclarationType> probe);

    // Expression generator methods
    void genAssign(std::shared_ptr<AST::AssignmentExprType> assign);
    void genUnaryPrefix(std::shared_ptr<AST::UnaryPrefixType> unaryExpr);
    void genMemberAccess(std::shared_ptr<AST::MemberExprType> expr);
    void genNumber(std::shared_ptr<AST::NumericLiteralType> num);
    void genString(std::shared_ptr<AST::StringLiteralType> string);
    void genIdent(std::shared_ptr<AST::IdentifierType> ident);
    void genCall(std::shared_ptr<AST::CallExprType> call);
    void genBinExpr(std::shared_ptr<AST::BinaryExprType> expr);
};

} // namespace Probescript