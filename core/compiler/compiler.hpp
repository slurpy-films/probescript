#pragma once

#include <string>
#include <vector>
#include <memory>

#include "frontend/ast.hpp"

#include "vm/builder.hpp"

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

    // Expression generator methods
    void genNumber(std::shared_ptr<AST::NumericLiteralType> num);
    void genIdent(std::shared_ptr<AST::IdentifierType> ident);
    void genCall(std::shared_ptr<AST::CallExprType> call);
    void genBinExpr(std::shared_ptr<AST::BinaryExprType> expr);
};

} // namespace Probescript