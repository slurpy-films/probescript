#pragma once
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"
#include <iostream>
#include <stdexcept>
#include "utils.hpp"
#include "errors.hpp"
#include "context.hpp"

template<typename T, typename... Args>
T* newnode(Lexer::Token tok, Args&&... args) {
    T* node = new T(std::forward<Args>(args)...);
    node->token = tok;
    return node;
}

class Parser
{
public:
    ProgramType* parse(std::string& sourceCode, std::shared_ptr<Context> ctx = std::make_shared<Context>());

private:
    std::vector<Lexer::Token> tokens;
    std::string file;
    std::shared_ptr<Context> context;

    // Statement methods

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

    Stmt* parseFunctionDeclaration(bool tkEaten = false);

    Stmt* parseIfStmt();

    VarDeclarationType* parseVarDeclaration(bool isConstant = false, bool tkEaten = false);

    // Expression methods

    Expr* parseExpr();

    Expr* parseAssignmentExpr();

    Expr* parseTernaryExpr();

    Expr* parseAsExpr();

    Expr* parseTemplateCall(Expr* caller);

    Expr* parseTemplateArg();

    Expr* parseLogicalExpr();

    Expr* parseEqualityExpr();

    Expr* parseRelationalExpr();

    Expr* parseObjectExpr();

    Expr* parseAdditiveExpr();

    Expr* parseMultiplicativeExpr();

    Expr* parseUnaryExpr();

    Expr* parseAwaitExpr();

    Expr* parseCallMemberExpr();

    Expr* parseMemberExpr();

    Expr* parseCallExpr(Expr* caller);

    Expr* parseArrowFunction();

    Expr* parseNewExpr();
        
    Expr* parseMemberChain(Expr* expr);

    Expr* parsePrimaryExpr();
    
    Expr* parseArrayExpr();

    // Utility methods

    std::vector<Expr*> parseArgs(bool braces = false);

    std::vector<Expr*> parseArgList();

    std::vector<VarDeclarationType*> parseParams();

    VarDeclarationType* parseParam();

    Lexer::Token at(int index = 0);

    Lexer::Token next();

    Lexer::Token eat();

    Lexer::Token expect(Lexer:: TokenType type, std::string err);

    bool notEOF();
    std::string getCurrentLine(Lexer::Token at);

    std::vector<Stmt*> parseBody(bool methods = false, std::string prbname = "");
};