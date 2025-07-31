#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

#include "frontend/ast.hpp"

#include "vm/builder.hpp"


namespace Probescript
{

class Compiler
{
public:
    Compiler(std::shared_ptr<AST::ProgramType> program, std::shared_ptr<Context> context)
        : m_program(program), builder(std::make_shared<VM::ByteCodeBuilder>()), m_context(context) {}

    void compile();

    std::vector<std::shared_ptr<VM::Instruction>> getInstructions();
    std::vector<VM::ValuePtr> getConstants();
private:
    std::shared_ptr<VM::ByteCodeBuilder> builder;
    std::shared_ptr<AST::ProgramType> m_program;

    std::shared_ptr<Context> m_context;

    std::vector<std::vector<size_t>> m_breakPatchesStack;
    std::vector<std::vector<size_t>> m_continuePatchesStack;
        
    void gen(std::shared_ptr<AST::Stmt> node);
    
    // Statement generator methods
    void genFunction(std::shared_ptr<AST::FunctionDeclarationType> fn, bool onlyValue = false);
    void genVarDecl(std::shared_ptr<AST::VarDeclarationType> decl);
    void genIf(std::shared_ptr<AST::IfStmtType> stmt);
    void genWhile(std::shared_ptr<AST::WhileStmtType> stmt);
    void genReturn(std::shared_ptr<AST::ReturnStmtType> stmt);
    void genProbe(std::shared_ptr<AST::ProbeDeclarationType> probe);
    void genFor(std::shared_ptr<AST::ForStmtType> forStmt);
    void genImport(std::shared_ptr<AST::ImportStmtType> stmt);
    void genBreak(std::shared_ptr<AST::BreakStmtType> breakStmt);
    void genContinue(std::shared_ptr<AST::ContinueStmtType> continueStmt);
    void genClass(std::shared_ptr<AST::ClassDefinitionType> cls);
    void genExport(std::shared_ptr<AST::ExportStmtType> exportStmt);
    void genTry(std::shared_ptr<AST::TryStmtType> tryStmt);

    // Expression generator methods
    void genAssign(std::shared_ptr<AST::AssignmentExprType> assign);
    void genMemberAssign(std::shared_ptr<AST::MemberAssignmentType> memberAssign);
    void genUnaryPrefix(std::shared_ptr<AST::UnaryPrefixType> unaryExpr);
    void genUnaryPostfix(std::shared_ptr<AST::UnaryPostFixType> unaryExpr);
    void genMemberAccess(std::shared_ptr<AST::MemberExprType> expr);
    void genNumber(std::shared_ptr<AST::NumericLiteralType> num);
    void genString(std::shared_ptr<AST::StringLiteralType> string);
    void genBoolean(std::shared_ptr<AST::BoolLiteralType> boolean);
    void genIdent(std::shared_ptr<AST::IdentifierType> ident);
    void genCall(std::shared_ptr<AST::CallExprType> call);
    void genBinExpr(std::shared_ptr<AST::BinaryExprType> expr);
    void genArrowFn(std::shared_ptr<AST::ArrowFunctionType> arrowFn);
    void genMapLiteral(std::shared_ptr<AST::MapLiteralType> map);
    void genNewExpr(std::shared_ptr<AST::NewExprType> expr);
    void genArrayLiteral(std::shared_ptr<AST::ArrayLiteralType> array);
    void genThrow(std::shared_ptr<AST::ThrowStmtType> stmt);

    void enterLoop();
    void exitLoop(size_t continueTarget, size_t breakTarget);
    bool isInLoop() const;
};

} // namespace Probescript
