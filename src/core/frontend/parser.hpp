#pragma once
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"
#include <iostream>
#include <stdexcept>
#include "utils.hpp"
#include "errors.hpp"
#include "context.hpp"

class Parser
{
public:
    std::shared_ptr<ProgramType> parse(std::string& sourceCode, std::shared_ptr<Context> ctx = std::make_shared<Context>());

private:
    std::vector<Lexer::Token> tokens;
    std::string file;
    std::shared_ptr<Context> context;

    // Statement methods

    std::shared_ptr<Stmt> parseStmt();

    std::shared_ptr<Stmt> parseProbeDeclaration();

    std::shared_ptr<Stmt> parseTryStmt();

    std::shared_ptr<Stmt> parseForStmt();
    
    std::shared_ptr<Stmt> parseThrowStmt();

    std::shared_ptr<Stmt> parseReturnStmt();

    std::shared_ptr<Stmt> parseClassDeclaration();

    std::shared_ptr<Stmt> parseModuleDeclaration();

    std::shared_ptr<Stmt> parseWhileStmt();

    std::shared_ptr<Stmt> parseImportStmt();

    std::shared_ptr<Stmt> parseExportStmt();

    std::shared_ptr<Stmt> parseFunctionDeclaration(bool tkEaten = false);

    std::shared_ptr<Stmt> parseIfStmt();

    std::shared_ptr<VarDeclarationType> parseVarDeclaration(bool isConstant = false, bool tkEaten = false);

    // Expression methods

    std::shared_ptr<Expr> parseExpr();

    std::shared_ptr<Expr> parseAssignmentExpr();

    std::shared_ptr<Expr> parseTernaryExpr();

    std::shared_ptr<Expr> parseAsExpr();

    std::shared_ptr<Expr> parseTemplateCall(std::shared_ptr<Expr> caller);

    std::shared_ptr<Expr> parseTemplateArg();

    std::shared_ptr<Expr> parseLogicalExpr();

    std::shared_ptr<Expr> parseEqualityExpr();

    std::shared_ptr<Expr> parseRelationalExpr();

    std::shared_ptr<Expr> parseObjectExpr();

    std::shared_ptr<Expr> parseAdditiveExpr();

    std::shared_ptr<Expr> parseMultiplicativeExpr();

    std::shared_ptr<Expr> parseUnaryExpr();

    std::shared_ptr<Expr> parseAwaitExpr();

    std::shared_ptr<Expr> parseCallMemberExpr();

    std::shared_ptr<Expr> parseMemberExpr();

    std::shared_ptr<Expr> parseCallExpr(std::shared_ptr<Expr> caller);

    std::shared_ptr<Expr> parseArrowFunction();

    std::shared_ptr<Expr> parseNewExpr();
        
    std::shared_ptr<Expr> parseMemberChain(std::shared_ptr<Expr> expr);

    std::shared_ptr<Expr> parsePrimaryExpr();
    
    std::shared_ptr<Expr> parseArrayExpr();

    // Utility methods

    std::vector<std::shared_ptr<Expr>> parseArgs();

    std::vector<std::shared_ptr<Expr>> parseArgList();

    std::vector<std::shared_ptr<VarDeclarationType>> parseParams();

    std::shared_ptr<VarDeclarationType> parseParam();

    Lexer::Token at(int index = 0);

    Lexer::Token next();

    Lexer::Token eat();

    Lexer::Token expect(Lexer:: TokenType type, std::string err);

    bool notEOF();
    std::string getCurrentLine(Lexer::Token at);

    std::vector<std::shared_ptr<Stmt>> parseBody(bool methods = false, std::string prbname = "");

    template<typename T, typename... Args>
    std::shared_ptr<T> newnode(Lexer::Token tok, Args&&... args) {
        std::shared_ptr<T> node = std::make_shared<T>(std::forward<Args>(args)...);
        tok.ctx = context;
        node->token = tok;
        return node;
    }
};