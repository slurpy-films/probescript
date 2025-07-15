#include "frontend/parser.hpp"

using Probescript::Lexer::Token;
using namespace Probescript;

std::shared_ptr<AST::ProgramType> Parser::parse(std::string& sourceCode, std::shared_ptr<Context> ctx)
{
    tokens = Lexer::tokenize(sourceCode);
    file = sourceCode;
    context = ctx;
    auto program = std::make_shared<AST::ProgramType>();

    while (notEOF())
    {
        program->body.push_back(parseStmt());
    }

    return program;
}

std::shared_ptr<AST::Stmt> Parser::parseStmt()
{
    std::shared_ptr<AST::Stmt> stmt;
    switch (at().type)
    {
        case Lexer::Probe:
            stmt = parseProbeDeclaration();
            break;
        case Lexer::Var:
            stmt = parseVarDeclaration();
            break;
        case Lexer::Const:
            stmt = parseVarDeclaration(true);
            break;
        case Lexer::Function:
            stmt = parseFunctionDeclaration();
            break;
        case Lexer::Async:
            stmt = parseFunctionDeclaration();
            break;
        case Lexer::Module:
            stmt = parseModuleDeclaration();
            break;
        case Lexer::If:
            stmt = parseIfStmt();
            break;
        case Lexer::Import:
            stmt = parseImportStmt();
            break;
        case Lexer::Return:
            stmt = parseReturnStmt();
            break;
        case Lexer::Export:
            stmt = parseExportStmt();
            break;
        case Lexer::While:
            stmt = parseWhileStmt();
            break;
        case Lexer::Class:
            stmt = parseClassDeclaration();
            break;
        case Lexer::For:
            stmt = parseForStmt();
            break;
        case Lexer::Throw:
            stmt = parseThrowStmt();
            break;
        case Lexer::Break:
            stmt = newnode<AST::BreakStmtType>(eat());
            break;
        case Lexer::Continue:
            stmt = newnode<AST::ContinueStmtType>(eat());
            break;
        case Lexer::Try:
            stmt = parseTryStmt();
            break;
        case Lexer::Semicolon:
            stmt = newnode<AST::UndefinedLiteralType>(eat());
            break;
        default:
            stmt = parseExpr();
    }

    if (at().type == Lexer::Semicolon) eat();
    return stmt;
}


std::shared_ptr<AST::Stmt> Parser::parseProbeDeclaration()
{
    Token token = eat();
    std::string name = expect(Lexer::Identifier, "Expected identifier").value;

    if (at().type == Lexer::Extends)
    {
        Token tk = eat();
        std::shared_ptr<AST::Expr> extends = parseExpr();
        std::vector<std::shared_ptr<AST::Stmt>> body = parseBody(true, name);

        return newnode<AST::ProbeDeclarationType>(tk, name, body, extends);
    }

    std::vector<std::shared_ptr<AST::Stmt>> body = parseBody(true, name);

    std::shared_ptr<AST::ProbeDeclarationType> prb = newnode<AST::ProbeDeclarationType>(token, name, body);

    return prb;
}

std::shared_ptr<AST::Stmt> Parser::parseTryStmt()
{
    Token tk = eat();
    std::vector<std::shared_ptr<AST::Stmt>> body = parseBody();

    Token catchtk = expect(Lexer::Catch, "Expected catch after try body");
    std::vector<std::shared_ptr<AST::VarDeclarationType>> params = parseParams();

    std::vector<std::shared_ptr<AST::Stmt>> catchBody = parseBody();

    return newnode<AST::TryStmtType>(tk, body, newnode<AST::FunctionDeclarationType>(catchtk, params, "catch", catchBody));
}

std::shared_ptr<AST::Stmt> Parser::parseForStmt()
{
    Token tk = eat();
    expect(Lexer::OpenParen, "Expected '(' after 'for'");

    std::vector<std::shared_ptr<AST::Stmt>> decl;
    while (at().type != Lexer::Semicolon && at().type != Lexer::END)
    {
        decl.push_back(parseVarDeclaration());
        if (at().type == Lexer::Comma) eat();
        else break;
    }

    expect(Lexer::Semicolon, "Expected semicolon after initializer in for loop");

    std::vector<std::shared_ptr<AST::Expr>> cond;
    while (at().type != Lexer::Semicolon && at().type != Lexer::END)
    {
        cond.push_back(parseExpr());
        if (at().type == Lexer::Comma) eat();
        else break;
    }

    expect(Lexer::Semicolon, "Expected semicolon after condition in for loop");

    std::vector<std::shared_ptr<AST::Expr>> update;
    while (at().type != Lexer::Semicolon && at().type != Lexer::END)
    {
        update.push_back(parseExpr());
        if (at().type == Lexer::Comma) eat();
        else break;
    }

    expect(Lexer::ClosedParen, "Expected closing parentheses after for loop updates");
    std::vector<std::shared_ptr<AST::Stmt>> body = parseBody();
    return newnode<AST::ForStmtType>(tk, decl, cond, update, body);
}

std::shared_ptr<AST::Stmt> Parser::parseThrowStmt()
{
    Token tk = eat();
    return newnode<AST::ThrowStmtType>(tk, parseExpr());
}

std::shared_ptr<AST::Stmt> Parser::parseReturnStmt()
{
    Token tk = eat();
    return newnode<AST::ReturnStmtType>(tk, at().type == Lexer::Semicolon ? std::make_shared<AST::UndefinedLiteralType>() : parseExpr());
}

std::shared_ptr<AST::Stmt> Parser::parseClassDeclaration()
{
    Token tk = eat();
    std::string name = expect(Lexer::Identifier, "Expected identifier").value;

    if (at().type == Lexer::Extends)
    {
        eat();
        std::shared_ptr<AST::Expr> extends = parseExpr();
        std::vector<std::shared_ptr<AST::Stmt>> body = parseBody(true);

        return newnode<AST::ClassDefinitionType>(tk, name, body, extends);
    }

    std::vector<std::shared_ptr<AST::Stmt>> body = parseBody(true);

    return newnode<AST::ClassDefinitionType>(tk, name, body);
}

std::shared_ptr<AST::Stmt> Parser::parseModuleDeclaration()
{
    Token tk = eat();
    expect(Lexer::Identifier, "Expected Identifier after module declaration");

    return newnode<AST::UndefinedLiteralType>(tk);
}

std::shared_ptr<AST::Stmt> Parser::parseWhileStmt()
{
    Token tk = eat();
    expect(Lexer::OpenParen, "Expected open parentheses after while keyword");

    std::shared_ptr<AST::Expr> condition = parseExpr();

    expect(Lexer::ClosedParen, "Expected closing parentheses after while condition");

    std::vector<std::shared_ptr<AST::Stmt>> body = parseBody();
    return newnode<AST::WhileStmtType>(tk, condition, body);
}

std::shared_ptr<AST::Stmt> Parser::parseImportStmt()
{   
    Token tk = eat();
    std::string name = at().value;
    if (at(1).type == Lexer::Dot)
    {
        std::shared_ptr<AST::Expr> module = parseExpr();

        if (at().type == Lexer::As)
        {
            eat();
            std::string ident = expect(Lexer::Identifier, "Expected identifier after as keyword").value;
            std::shared_ptr<AST::ImportStmtType> importstmt = newnode<AST::ImportStmtType>(tk, name, module, ident);
            return importstmt;
        }

        std::shared_ptr<AST::ImportStmtType> importstmt = newnode<AST::ImportStmtType>(tk, name, module);
        return importstmt;
    } else eat();

    if (at().type == Lexer::As)
    {
        eat();
        std::string ident = expect(Lexer::Identifier, "Expected identifier after as keyword").value;
        std::shared_ptr<AST::ImportStmtType> importstmt = newnode<AST::ImportStmtType>(tk, name, ident);
        return importstmt;
    }

    std::shared_ptr<AST::ImportStmtType> importstmt = newnode<AST::ImportStmtType>(tk, name);

    return importstmt;
}

std::shared_ptr<AST::Stmt> Parser::parseExportStmt()
{
    Token tk = eat();
    std::shared_ptr<AST::Stmt> value = parseStmt();

    std::shared_ptr<AST::ExportStmtType> exportstmt = newnode<AST::ExportStmtType>(tk, value);

    return exportstmt;
}

std::shared_ptr<AST::Stmt> Parser::parseFunctionDeclaration(bool tkEaten)
{
    Token tk = at();
    if (!tkEaten) eat();
    
    bool isAsync = tk.type == Lexer::Async;
    std::string name = (at().type == Lexer::Identifier || at().type == Lexer::New ? eat().value : "anonymous");
    std::vector<std::shared_ptr<AST::VarDeclarationType>> templateparams;

    if (at().value == "<")
    {
        eat();
        while (at().value != ">" && notEOF())
        {
            Token ident = expect(Lexer::Identifier, "Expected identifier");
            std::shared_ptr<AST::Expr> value;

            if (at().type == Lexer::Equals)
            {
                eat();
                value = parseExpr();
            }

            templateparams.push_back(newnode<AST::VarDeclarationType>(ident, value, ident.value, isAsync));

            if (at().type == Lexer::Comma) eat();
        }
        eat();
    }

    std::vector<std::shared_ptr<AST::VarDeclarationType>> params = parseParams();

    std::shared_ptr<AST::Expr> type = nullptr;

    if (at().type == Lexer::Colon)
    {
        eat();

        type = parseExpr();
    }

    std::vector<std::shared_ptr<AST::Stmt>> body = parseBody();

    std::shared_ptr<AST::FunctionDeclarationType> fn = (type ? newnode<AST::FunctionDeclarationType>(tk, params, name, body, type, isAsync) : newnode<AST::FunctionDeclarationType>(tk, params, name, body, isAsync));

    fn->templateparams = templateparams;

    return fn;
}

std::shared_ptr<AST::Stmt> Parser::parseIfStmt()
{
    Token tk = eat();
    expect(Lexer::OpenParen, "Expected opening parethesis");

    std::shared_ptr<AST::Expr> condition = parseExpr();

    Token lastToken = eat();
    if (lastToken.type != Lexer::ClosedParen)
    {
        throw std::runtime_error(SyntaxError("Expected closing parentheses, recieved " + lastToken.value, lastToken, context));
    }

    std::vector<std::shared_ptr<AST::Stmt>> body = parseBody();

    std::shared_ptr<AST::IfStmtType> ifStmt = newnode<AST::IfStmtType>(tk, condition, body);

    if (at().type == Lexer::Else)
    {
        eat();
        ifStmt->elseStmt = parseBody();
        ifStmt->hasElse = true;
    }

    return ifStmt;
}

std::shared_ptr<AST::VarDeclarationType> Parser::parseVarDeclaration(bool isConstant, bool tkEaten)
{
    Token tk = at();
    if (!tkEaten) eat();
    std::string ident = expect(Lexer::Identifier, "Expected identifier").value;

    bool hasType = false;
    std::shared_ptr<AST::Expr> type;

    if (at().type == Lexer::Colon)
    {
        eat();
        type = parseLogicalExpr();
        hasType = true;
    }

    if (at().type != Lexer::Equals)
    {
        if (isConstant)
        {
            throw std::runtime_error(SyntaxError("Must assign value to constant variable", at(), context));
        }

        if (!hasType)
            return newnode<AST::VarDeclarationType>(tk, std::make_shared<AST::UndefinedLiteralType>(), ident);
        else
            return newnode<AST::VarDeclarationType>(tk, std::make_shared<AST::UndefinedLiteralType>(), ident, type);
    }
    
    eat();
    if (!hasType)
        return newnode<AST::VarDeclarationType>(tk, parseExpr(), ident);
    else
        return newnode<AST::VarDeclarationType>(tk, parseExpr(), ident, type);
}

std::shared_ptr<AST::Expr> Parser::parseExpr()
{
    return parseAssignmentExpr();
}

std::shared_ptr<AST::Expr> Parser::parseAssignmentExpr()
{
    std::shared_ptr<AST::Expr> left = parseTernaryExpr();

    if (at().type == Lexer::Increment || at().type == Lexer::Decrement)
    {
        Token op = eat();
        return newnode<AST::UnaryPostFixType>(op, op.value, left);
    }

    if (at().type == Lexer::Equals || at().type == Lexer::AssignmentOperator)
    {
        Token op = eat();
        std::shared_ptr<AST::Expr> value = parseExpr();
        if (left->kind == AST::NodeType::MemberExpr)
        {
            std::shared_ptr<AST::MemberExprType> expr = std::static_pointer_cast<AST::MemberExprType>(left);
            return newnode<AST::MemberAssignmentType>(op, expr->object, expr->property, value, expr->computed, op.value);
        }
        else
        {
            return newnode<AST::AssignmentExprType>(op, left, value, op.value);
        }
    }

    return left;
}

std::shared_ptr<AST::Expr> Parser::parseTernaryExpr()
{
    std::shared_ptr<AST::Expr> cond = parseAsExpr();

    if (at().type != Lexer::Ternary) return cond;

    Token tk = eat();

    std::shared_ptr<AST::Expr> cons = parseExpr();
    expect(Lexer::Colon, "Expected colon after ternary consequent");
    std::shared_ptr<AST::Expr> alt = parseExpr();

    return newnode<AST::TernaryExprType>(tk, cond, cons, alt);
}

std::shared_ptr<AST::Expr> Parser::parseAsExpr()
{
    std::shared_ptr<AST::Expr> left = parseLogicalExpr();

    while (at().type == Lexer::As && at().type != Lexer::END)
    {
        Token tk = eat();
        left = newnode<AST::CastExprType>(tk, left, parseExpr());
    }

    return left;
}

std::shared_ptr<AST::Expr> Parser::parseTemplateCall(std::shared_ptr<AST::Expr> caller)
{
    Token open = expect(Lexer::LessThan, "Expected '<'");
    std::vector<std::shared_ptr<AST::Expr>> tempargs;

    if (at().type != Lexer::GreaterThan)
    {
        tempargs.push_back(parseTemplateArg());

        if (at().type != Lexer::Comma && at().type != Lexer::GreaterThan && at().type != Lexer::Equals) 
        {
            return newnode<AST::BinaryExprType>(caller->token, caller, tempargs[0], "<");
        }

        while (at().type == Lexer::Comma)
        {
            eat();
            tempargs.push_back(parseTemplateArg());
        }
    }

    expect(Lexer::GreaterThan, "Expected '>' after template arguments");

    std::shared_ptr<AST::Expr> call = newnode<AST::TemplateCallType>(open, caller, tempargs);

    if (at().type == Lexer::OpenParen)
    {
        call = parseCallExpr(call);
    }

    return call;
}

std::shared_ptr<AST::Expr> Parser::parseTemplateArg()
{
    return parseObjectExpr();
}

std::shared_ptr<AST::Expr> Parser::parseLogicalExpr()
{
    std::shared_ptr<AST::Expr> left = parseEqualityExpr();

    while (at().type == Lexer::AndOperator || at().type == Lexer::OrOperator)
    {
        Token op = eat();
        std::shared_ptr<AST::Expr> right = parseEqualityExpr();
        left = newnode<AST::BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

std::shared_ptr<AST::Expr> Parser::parseEqualityExpr()
{
    std::shared_ptr<AST::Expr> left = parseRelationalExpr();

    while (at().type == Lexer::DoubleEquals || at().type == Lexer::NotEquals)
    {
        Token op = eat();

        std::shared_ptr<AST::Expr> right = parseRelationalExpr();
        left = newnode<AST::BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

std::shared_ptr<AST::Expr> Parser::parseRelationalExpr()
{
    std::shared_ptr<AST::Expr> left = parseObjectExpr();

    while (at().type == Lexer::LessThan || at().type == Lexer::GreaterThan || at().value == "<=" || at().value == ">=")
    {
        Token op = eat();

        std::shared_ptr<AST::Expr> right = parseExpr();

        left = newnode<AST::BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

std::shared_ptr<AST::Expr> Parser::parseObjectExpr()
{
    if (at().type != Lexer::OpenBrace)
    {
        return parseAdditiveExpr();
    }

    Token tk = eat();

    std::vector<std::shared_ptr<AST::PropertyLiteralType>> properties;

    while (notEOF() && at().type != Lexer::ClosedBrace)
    {
        Token key = eat();

        if (at().type == Lexer::Comma)
        {
            eat();
            properties.push_back(newnode<AST::PropertyLiteralType>(key, key.value, std::make_shared<AST::UndefinedLiteralType>()));
            continue;
        } else if (at().type == Lexer::ClosedBrace)
        {
            properties.push_back(newnode<AST::PropertyLiteralType>(key, key.value, std::make_shared<AST::UndefinedLiteralType>()));
            continue;
        }

        expect(Lexer::Colon, "Expected colon");

        std::shared_ptr<AST::Expr> value = parseExpr();

        properties.push_back(newnode<AST::PropertyLiteralType>(key, key.value, value));

        if (at().type != Lexer::ClosedBrace)
        {
            expect(Lexer::Comma, "Expected comma");
        }
    }

    expect(Lexer::ClosedBrace, "Object literal missing closing bracket");

    return newnode<AST::MapLiteralType>(tk, properties);
}

std::shared_ptr<AST::Expr> Parser::parseAdditiveExpr()
{
    std::shared_ptr<AST::Expr> left = parseMultiplicativeExpr();
    while (at().value == "+" || at().value == "-")
    {
        Token op = eat();
        std::shared_ptr<AST::Expr> right = parseMultiplicativeExpr();
        left = newnode<AST::BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

std::shared_ptr<AST::Expr> Parser::parseMultiplicativeExpr()
{
    std::shared_ptr<AST::Expr> left = parseUnaryExpr();

    while (at().value == "*" || at().value == "/" || at().value == "%")
    {
        Token op = eat();
        std::shared_ptr<AST::Expr> right = parseUnaryExpr();
        left = newnode<AST::BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

std::shared_ptr<AST::Expr> Parser::parseUnaryExpr()
{
    if (at().type == Lexer::Bang ||
        at().type == Lexer::Increment || 
        at().type == Lexer::Decrement
    )
    {
        Token op = eat();
        std::shared_ptr<AST::Expr> argument = parseExpr();
        return newnode<AST::UnaryPrefixType>(op, op.value, argument);
    }

    return parseAwaitExpr();
}

std::shared_ptr<AST::Expr> Parser::parseAwaitExpr()
{
    if (at().type != Lexer::Await)
        return parseCallMemberExpr();

    Token tk = eat();
    return newnode<AST::AwaitExprType>(tk, parseCallMemberExpr());
}

std::shared_ptr<AST::Expr> Parser::parseCallExpr(std::shared_ptr<AST::Expr> caller)
{
    if (at().type == Lexer::LessThan)
    {
        caller = parseTemplateCall(caller);
    }

    std::shared_ptr<AST::CallExprType> callExpr = newnode<AST::CallExprType>(at(), caller, parseArgs());
    if (at().type == Lexer::OpenParen) return parseCallExpr(callExpr);
    
    if (at().type == Lexer::Dot || at().type == Lexer::OpenBracket)
    {
        return parseMemberChain(callExpr);
    }
    
    return callExpr;
}

std::shared_ptr<AST::Expr> Parser::parseCallMemberExpr()
{
    std::shared_ptr<AST::Expr> member = parseMemberExpr();

    if (at().type == Lexer::OpenParen)
    {
        return parseCallExpr(member);
    } else return member;
}

std::shared_ptr<AST::Expr> Parser::parseMemberExpr()
{
    std::shared_ptr<AST::Expr> obj = parseArrowFunction();

    while (at().type == Lexer::Dot || at().type == Lexer::OpenBracket || at().type == Lexer::LessThan)
    {
        if (at().type == Lexer::LessThan)
        {
            obj = parseTemplateCall(obj);
            continue;
        }

        Token op = eat();
        std::shared_ptr<AST::Expr> property;
        bool computed;
        std::string lastProp;

        if (op.type == Lexer::Dot)
        {
            computed = false;
            property = parsePrimaryExpr();

            if (property->kind != AST::NodeType::Identifier)
            {
                throw std::runtime_error(SyntaxError("Cannot use dot operator without right hand side being an identifier", op, context));
            }

            lastProp = std::static_pointer_cast<AST::IdentifierType>(property)->symbol;

        } else
        {
            computed = true;
            property = parseExpr();
            expect(Lexer::CloseBracket, "Expected closing bracket");

            if (property->kind == AST::NodeType::StringLiteral)
            {
                lastProp = std::static_pointer_cast<AST::StringLiteralType>(property)->strValue;
            }
        }

        std::shared_ptr<AST::MemberExprType> member = newnode<AST::MemberExprType>(op, obj, property, computed);
        member->lastProp = lastProp;
        obj = member;
    }

    return obj;
}

std::shared_ptr<AST::Expr> Parser::parseArrowFunction()
{
    if (at().type != Lexer::Function)
    {
        return parseNewExpr();
    }

    Token tk = eat();

    std::vector<std::shared_ptr<AST::VarDeclarationType>> params = parseParams();

    if (at().type == Lexer::Arrow)
        eat();

    std::vector<std::shared_ptr<AST::Stmt>> body;
    if (at().type == Lexer::OpenBrace)
    {
        body = parseBody();
    }
    else
    {
        body.push_back(newnode<AST::ReturnStmtType>(tk, parseExpr()));
    }

    return newnode<AST::ArrowFunctionType>(tk, params, body);
}

std::shared_ptr<AST::Expr> Parser::parseNewExpr()
{
    if (at().type != Lexer::New)
    {
        return parsePrimaryExpr();
    }

    Token tk = eat();
    std::vector<std::shared_ptr<AST::Expr>> args;

    std::shared_ptr<AST::Expr> constructor = parseMemberExpr();
    if (constructor->kind == AST::NodeType::CallExpr)
    {
        std::shared_ptr<AST::CallExprType> call = std::static_pointer_cast<AST::CallExprType>(constructor);
        args = call->args;
        constructor = call->calee;
    }
    else
    {
        args = parseArgs();
    }
    
    return newnode<AST::NewExprType>(tk, constructor, args);
}

std::shared_ptr<AST::Expr> Parser::parseMemberChain(std::shared_ptr<AST::Expr> expr)
{
    while (at().type == Lexer::Dot || at().type == Lexer::OpenBracket)
    {
        Token op = eat();
        std::shared_ptr<AST::Expr> property;
        bool computed;
        std::string lastProp;
        
        if (op.type == Lexer::Dot)
        {
            computed = false;
            property = parsePrimaryExpr();
            
            if (property->kind != AST::NodeType::Identifier)
            {
                throw std::runtime_error(SyntaxError("Cannot use dot operator without right hand side being an identifier", op, context));
            }
            
            lastProp = std::static_pointer_cast<AST::IdentifierType>(property)->symbol;
            
        } else
        {
            computed = true;
            property = parseExpr();
            expect(Lexer::CloseBracket, "Expected closing bracket");
            
            if (property->kind == AST::NodeType::StringLiteral)
            {
                lastProp = std::static_pointer_cast<AST::StringLiteralType>(property)->strValue;
            }
        }
        
        std::shared_ptr<AST::MemberExprType> member = newnode<AST::MemberExprType>(op, expr, property, computed);
        member->lastProp = lastProp;
        expr = member;
        
        if (at().type == Lexer::OpenParen)
        {
            expr = parseCallExpr(expr);
        }
    }
    
    return expr;
}

std::shared_ptr<AST::Expr> Parser::parsePrimaryExpr()
{
    Token tk = at();
    if (tk.type == Lexer::OpenBracket)
    {
        return parseArrayExpr();
    }

    std::shared_ptr<AST::Expr> primary;

    switch (tk.type)
    {
        case Lexer::Identifier:
        {
            Token token = eat();
            primary = newnode<AST::IdentifierType>(token, token.value);
            break;
        }

        case Lexer::Number:
        {
            Token token = eat();
            primary = newnode<AST::NumericLiteralType>(token, std::stod(token.value));
            break;
        }

        case Lexer::String:
        {
            Token token = eat();
            primary = newnode<AST::StringLiteralType>(token, token.value);
            break;
        }

        case Lexer::OpenParen:
        {
            eat();
            std::shared_ptr<AST::Expr> value = parseExpr();
            expect(Lexer::ClosedParen, "Expected closing parentheses ");
            primary = value;
            break;
        }

        case Lexer::Bool:
            primary = newnode<AST::BoolLiteralType>(at(), eat().value == "true");
            break;

        case Lexer::Undefined:
            primary = newnode<AST::UndefinedLiteralType>(eat());
            break;

        case Lexer::Null:
            primary = newnode<AST::NullLiteralType>(eat());
            break;

        case Lexer::END:
            primary = newnode<AST::UndefinedLiteralType>(eat());
            break;

        default:
            throw std::runtime_error(SyntaxError("Unexpected token found while parsing: " + at().value, at(), context));
    }

    if (primary->kind == AST::NodeType::Identifier && at().type == Lexer::LessThan)
    {
        primary = parseTemplateCall(primary);
    }

    return primary;
}

std::shared_ptr<AST::Expr> Parser::parseArrayExpr()
{
    if (at().type != Lexer::OpenBracket)
    {
        return parseCallMemberExpr();
    }

    Token tk = eat();

    std::vector<std::shared_ptr<AST::Expr>> items;

    if (at().type == Lexer::CloseBracket)
    {
        eat();
        return newnode<AST::ArrayLiteralType>(tk, items);
    }
    
    items.push_back(parseExpr());

    while (at().type == Lexer::Comma)
    {
        eat();
        items.push_back(parseExpr());
    }

    expect(Lexer::CloseBracket, "Expected closing bracket");

    return newnode<AST::ArrayLiteralType>(tk, items);
}

std::vector<std::shared_ptr<AST::Expr>> Parser::parseArgs()
{
    expect(Lexer::OpenParen, "Expected open parentheses");

    std::vector<std::shared_ptr<AST::Expr>> args; 
    if (at().type != (Lexer::ClosedParen))
    {
        args = parseArgList();
    }

    expect(Lexer::ClosedParen, "Expected closing parentheses");
    
    return args;
}

std::vector<std::shared_ptr<AST::Expr>> Parser::parseArgList()
{
    std::vector<std::shared_ptr<AST::Expr>> args;

    args.push_back(parseAssignmentExpr());

    while (notEOF() && at().type == Lexer::Comma)
    {
        eat();
        args.push_back(parseAssignmentExpr());
    }

    return args;
}

std::shared_ptr<AST::VarDeclarationType> Parser::parseParam() {
    Token ident = expect(Lexer::Identifier, "Expected identifier");

    std::shared_ptr<AST::Expr> type = nullptr;
    std::shared_ptr<AST::Expr> value = nullptr;

    if (at().type == Lexer::Colon) {
        eat();
        type = parseExpr();
    }

    if (at().type == Lexer::Equals) {
        Token eq = eat();
        value = parseExpr();
        return newnode<AST::VarDeclarationType>(eq, value, ident.value, type ? type : std::make_shared<AST::UndefinedLiteralType>());
    }

    return newnode<AST::VarDeclarationType>(ident, std::make_shared<AST::UndefinedLiteralType>(), ident.value, type);
}

std::vector<std::shared_ptr<AST::VarDeclarationType>> Parser::parseParams() {
    expect(Lexer::OpenParen, "Expected parentheses before parameters");
    std::vector<std::shared_ptr<AST::VarDeclarationType>> params;

    if (at().type == Lexer::ClosedParen) {
        eat();
        return params;
    }

    params.push_back(parseParam());

    while (at().type == Lexer::Comma) {
        eat();
        params.push_back(parseParam());
    }

    expect(Lexer::ClosedParen, "Expected closing parentheses after parameters");
    return params;
}

std::vector<std::shared_ptr<AST::Stmt>> Parser::parseBody(bool methods, std::string prbname)
{
    if (at().type == Lexer::OpenBrace)
    {
        eat();
        std::vector<std::shared_ptr<AST::Stmt>> body;
        if (methods)
        {
            while (at().type != Lexer::ClosedBrace && at().type != Lexer::END)
            {
                if (at().type == Lexer::Identifier || at().type == Lexer::New)
                {
                    std::string name = at().value;
                    if (at(1).type == Lexer::OpenParen || at(1).type == Lexer::LessThan)
                    {
                        std::shared_ptr<AST::Stmt> fn = parseFunctionDeclaration(true);

                        if (fn->kind == AST::NodeType::FunctionDeclaration)
                        {
                            std::shared_ptr<AST::FunctionDeclarationType> func = std::static_pointer_cast<AST::FunctionDeclarationType>(fn);
                            
                            func->name = (func->name == prbname ? "run" : func->name);

                            body.push_back(fn);
                        } else body.push_back(fn);
                    } else
                    {
                        body.push_back(parseVarDeclaration(false, true));
                    }
                } else body.push_back(parseStmt());
            }
        } else
        {
            while (at().type != Lexer::ClosedBrace && at().type != Lexer::END)
            {
                body.push_back(parseStmt());
            }
        }

        expect(Lexer::ClosedBrace, "Expected closing brace");
        return body;
    } else
    {
        std::vector<std::shared_ptr<AST::Stmt>> body;
        body.push_back(parseStmt());
        return body;
    }
}

Lexer::Token Parser::at(int index)
{
    return tokens[index];
}

Lexer::Token Parser::next()
{
    return tokens[1];
}

Lexer::Token Parser::eat()
{
    Lexer::Token prev = shift(tokens);
    return prev;
}

Lexer::Token Parser::expect(Lexer::TokenType type, std::string err)
{
    Lexer::Token prev = shift(tokens);
    if (prev.type != type)
    {
        throw std::runtime_error(SyntaxError(err, prev, context));
    }

    return prev;
}

std::string Parser::getCurrentLine(Lexer::Token at)
{
    return split(file, "\n")[at.line - 1];
}

bool Parser::notEOF() {
    return tokens[0].type != Lexer::END;
}