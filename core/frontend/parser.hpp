#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <memory>

#include "utils.hpp"
#include "errors.hpp"
#include "context.hpp"
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"

namespace Probescript
{

class Parser
{
public:
    std::shared_ptr<AST::ProgramType> parse(std::string& sourceCode, std::shared_ptr<Context> ctx = std::make_shared<Context>());

private:
    std::vector<Lexer::Token> tokens;
    std::string file;
    std::shared_ptr<Context> context;

    // Statement methods

    std::shared_ptr<AST::Stmt> parseStmt();

    std::shared_ptr<AST::Stmt> parseProbeDeclaration();

    std::shared_ptr<AST::Stmt> parseTryStmt();

    std::shared_ptr<AST::Stmt> parseForStmt();
    
    std::shared_ptr<AST::Stmt> parseThrowStmt();

    std::shared_ptr<AST::Stmt> parseReturnStmt();

    std::shared_ptr<AST::Stmt> parseClassDeclaration();

    std::shared_ptr<AST::Stmt> parseModuleDeclaration();

    std::shared_ptr<AST::Stmt> parseWhileStmt();

    std::shared_ptr<AST::Stmt> parseImportStmt();

    std::shared_ptr<AST::Stmt> parseExportStmt();

    std::shared_ptr<AST::Stmt> parseFunctionDeclaration(bool tkEaten = false);

    std::shared_ptr<AST::Stmt> parseIfStmt();

    std::shared_ptr<AST::VarDeclarationType> parseVarDeclaration(bool isConstant = false, bool tkEaten = false);

    // Expression methods

    std::shared_ptr<AST::Expr> parseExpr();

    std::shared_ptr<AST::Expr> parseAssignmentExpr();

    std::shared_ptr<AST::Expr> parseTernaryExpr();

    std::shared_ptr<AST::Expr> parseAsExpr();

    std::shared_ptr<AST::Expr> parseTemplateCall(std::shared_ptr<AST::Expr> caller);

    std::shared_ptr<AST::Expr> parseTemplateArg();

    std::shared_ptr<AST::Expr> parseLogicalExpr();

    std::shared_ptr<AST::Expr> parseEqualityExpr();

    std::shared_ptr<AST::Expr> parseRelationalExpr();

    std::shared_ptr<AST::Expr> parseObjectExpr();

    std::shared_ptr<AST::Expr> parseAdditiveExpr();

    std::shared_ptr<AST::Expr> parseMultiplicativeExpr();

    std::shared_ptr<AST::Expr> parseUnaryExpr();

    std::shared_ptr<AST::Expr> parseAwaitExpr();

    std::shared_ptr<AST::Expr> parseCallMemberExpr();

    std::shared_ptr<AST::Expr> parseMemberExpr();

    std::shared_ptr<AST::Expr> parseCallExpr(std::shared_ptr<AST::Expr> caller);

    std::shared_ptr<AST::Expr> parseArrowFunction();

    std::shared_ptr<AST::Expr> parseNewExpr();
        
    std::shared_ptr<AST::Expr> parseMemberChain(std::shared_ptr<AST::Expr> expr);

    std::shared_ptr<AST::Expr> parsePrimaryExpr();
    
    std::shared_ptr<AST::Expr> parseArrayExpr();

    // Utility methods

    std::vector<std::shared_ptr<AST::Expr>> parseArgs();

    std::vector<std::shared_ptr<AST::Expr>> parseArgList();

    std::vector<std::shared_ptr<AST::VarDeclarationType>> parseParams();

    std::shared_ptr<AST::VarDeclarationType> parseParam();

    Lexer::Token at(int index = 0);

    Lexer::Token next();

    Lexer::Token eat();

    Lexer::Token expect(Lexer:: TokenType type, std::string err);

    bool notEOF();
    std::string getCurrentLine(Lexer::Token at);

    std::vector<std::shared_ptr<AST::Stmt>> parseBody(bool methods = false, std::string prbname = "");

    template<typename T, typename... Args>
    std::shared_ptr<T> newnode(Lexer::Token tok, Args&&... args) {
        std::shared_ptr<T> node = std::make_shared<T>(std::forward<Args>(args)...);
        tok.ctx = context;
        node->token = tok;
        return node;
    }
};

}