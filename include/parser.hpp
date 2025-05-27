#pragma once
#include "ast.hpp"
#include "lexer.hpp"
#include <iostream>
#include "utils/shift.hpp"
#include "utils/split.hpp"
#include "errors.hpp"
#include "context.hpp"

class Parser
{
public:
    ProgramType* produceAST(std::string& sourceCode, Context* ctx = new Context());

private:
    std::vector<Lexer::Token> tokens;
    std::string file;
    Context* context;

    // Statment methods

    Stmt* parseStmt();

    Stmt* parseProbeDeclaration();

    Stmt* parseTryStmt();

    Stmt* parseForStmt();
    
    Stmt* parseThrowStmt();

    Stmt* parseReturnStmt();

    Stmt* parseClassDeclaration();

    Stmt* parseModuleDeclaration();

    Stmt* parseWhileStmt();

    Stmt* parseImportStmt();

    Stmt* parseExportStmt();

    Stmt* parseFunctionDeclaration();

    Stmt* parseIfStmt();

    VarDeclarationType* parseVarDeclaration(bool isConstant = false);

    // Expression methods

    Expr* parseExpr();

    Expr* parseAssignmentExpr();

    Expr* parseLogicalExpr();

    Expr* parseEqualityExpr();

    Expr* parseRelationalExpr();

    Expr* parseObjectExpr();

    Expr* parseAdditiveExpr();

    Expr* parseMultiplicativeExpr();

    Expr* parseUnaryExpr();

    Expr* parseCallMemberExpr();

    Expr* parseCallexpr(Expr* caller);

    Expr* parseArrowFunction();

    Expr* parseNewExpr();
    
    Expr* parseMemberExpr();
    
    Expr* parseMemberChain(Expr* expr);

    Expr* parsePrimaryExpr();
    
    Expr* parseArrayExpr();

    // Utility methods

    std::vector<Expr*> parseArgs();

    std::vector<Expr*> parseArgList();

    std::vector<VarDeclarationType*> parseParams();

    Lexer::Token at(int index = 0);

    Lexer::Token next();

    Lexer::Token eat();

    Lexer::Token expect(Lexer:: TokenType type, std::string err);

    bool notEOF();
    std::string getCurrentLine(Lexer::Token at);

    std::vector<Stmt*> parseBody(bool methods = false, std::string prbname = "");
};