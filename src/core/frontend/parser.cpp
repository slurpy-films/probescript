#include "frontend/parser.hpp"

using Lexer::Token;

ProgramType* Parser::produceAST(std::string& sourceCode, std::shared_ptr<Context> ctx)
{
    tokens = Lexer::tokenize(sourceCode);
    file = sourceCode;
    context = ctx;
    ProgramType* program = new ProgramType();

    while (notEOF())
    {
        program->body.push_back(parseStmt());
    }

    return program;
}

Stmt* Parser::parseStmt()
{
    Stmt* stmt;
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
            stmt = newNode<BreakStmtType>(eat());
            break;
        case Lexer::Continue:
            stmt = newNode<ContinueStmtType>(eat());
            break;
        case Lexer::Try:
            stmt = parseTryStmt();
            break;
        case Lexer::Semicolon:
            stmt = newNode<UndefinedLiteralType>(eat());
            break;
        default:
            stmt = parseExpr();
    }

    if (at().type == Lexer::Semicolon) eat();
    return stmt;
}


Stmt* Parser::parseProbeDeclaration()
{
    Token token = eat();
    std::string name = expect(Lexer::Identifier, "Expected identifier").value;

    if (at().type == Lexer::Extends)
    {
        Token tk = eat();
        Expr* extends = parseExpr();
        std::vector<Stmt*> body = parseBody(true, name);

        return newNode<ProbeDeclarationType>(tk, name, body, extends);
    }

    std::vector<Stmt*> body = parseBody(true, name);

    ProbeDeclarationType* prb = newNode<ProbeDeclarationType>(token, name, body);

    return prb;
}

Stmt* Parser::parseTryStmt()
{
    Token tk = eat();
    std::vector<Stmt*> body = parseBody();

    Token catchtk = expect(Lexer::Catch, "Expected catch after try body");
    std::vector<VarDeclarationType*> params = parseParams();

    std::vector<Stmt*> catchBody = parseBody();

    return newNode<TryStmtType>(tk, body, newNode<FunctionDeclarationType>(catchtk, params, "catch", catchBody));
}

Stmt* Parser::parseForStmt()
{
    Token tk = eat();
    expect(Lexer::OpenParen, "Expected '(' after 'for'");

    std::vector<Stmt*> decl;
    while (at().type != Lexer::Semicolon && at().type != Lexer::END)
    {
        decl.push_back(parseVarDeclaration());
        if (at().type == Lexer::Comma) eat();
        else break;
    }

    expect(Lexer::Semicolon, "Expected semicolon after initializer in for loop");

    std::vector<Expr*> cond;
    while (at().type != Lexer::Semicolon && at().type != Lexer::END)
    {
        cond.push_back(parseExpr());
        if (at().type == Lexer::Comma) eat();
        else break;
    }

    expect(Lexer::Semicolon, "Expected semicolon after condition in for loop");

    std::vector<Expr*> update;
    while (at().type != Lexer::Semicolon && at().type != Lexer::END)
    {
        update.push_back(parseExpr());
        if (at().type == Lexer::Comma) eat();
        else break;
    }

    expect(Lexer::ClosedParen, "Expected closing parentheses after for loop updates");
    std::vector<Stmt*> body = parseBody();
    return newNode<ForStmtType>(tk, decl, cond, update, body);
}

Stmt* Parser::parseThrowStmt()
{
    Token tk = eat();
    return newNode<ThrowStmtType>(tk, parseExpr());
}

Stmt* Parser::parseReturnStmt()
{
    Token tk = eat();
    return newNode<ReturnStmtType>(tk, parseExpr());
}

Stmt* Parser::parseClassDeclaration()
{
    Token tk = eat();
    std::string name = expect(Lexer::Identifier, "Expected identifier").value;

    if (at().type == Lexer::Extends)
    {
        eat();
        Expr* extends = parseExpr();
        std::vector<Stmt*> body = parseBody(true);

        return newNode<ClassDefinitionType>(tk, name, body, extends);
    }

    std::vector<Stmt*> body = parseBody(true);

    return newNode<ClassDefinitionType>(tk, name, body);
}

Stmt* Parser::parseModuleDeclaration()
{
    Token tk = eat();
    expect(Lexer::Identifier, "Expected Identifier after module declaration");

    return newNode<UndefinedLiteralType>(tk);
}

Stmt* Parser::parseWhileStmt()
{
    Token tk = eat();
    expect(Lexer::OpenParen, "Expected open parentheses after while keyword");

    Expr* condition = parseExpr();

    expect(Lexer::ClosedParen, "Expected closing parentheses after while condition");

    std::vector<Stmt*> body = parseBody();
    return newNode<WhileStmtType>(tk, condition, body);
}

Stmt* Parser::parseImportStmt()
{   
    Token tk = eat();
    std::string name = at().value;
    if (at(1).type == Lexer::Dot)
    {
        Expr* module = parseExpr();

        if (at().type == Lexer::As)
        {
            eat();
            std::string ident = expect(Lexer::Identifier, "Expected identifier after as keyword").value;
            ImportStmtType* importstmt = newNode<ImportStmtType>(tk, name, module, ident);
            return importstmt;
        }

        ImportStmtType* importstmt = newNode<ImportStmtType>(tk, name, module);
        return importstmt;
    } else eat();

    if (at().type == Lexer::As)
    {
        eat();
        std::string ident = expect(Lexer::Identifier, "Expected identifier after as keyword").value;
        ImportStmtType* importstmt = newNode<ImportStmtType>(tk, name, ident);
        return importstmt;
    }

    ImportStmtType* importstmt = newNode<ImportStmtType>(tk, name);

    return importstmt;
}

Stmt* Parser::parseExportStmt()
{
    Token tk = eat();
    Stmt* value = parseStmt();

    ExportStmtType* exportstmt = newNode<ExportStmtType>(tk, value);

    return exportstmt;
}

Stmt* Parser::parseFunctionDeclaration(bool tkEaten)
{
    Token tk = at();
    if (!tkEaten) eat();
    std::string name = (at().type == Lexer::Identifier ? eat().value : "anonymous");

    std::vector<VarDeclarationType*> params = parseParams();

    Expr* type = nullptr;

    if (at().type == Lexer::Colon)
    {
        eat();

        type = parseExpr();
    }

    std::vector<Stmt*> body = parseBody();

    FunctionDeclarationType* fn = (type ? newNode<FunctionDeclarationType>(tk, params, name, body, type) : newNode<FunctionDeclarationType>(tk, params, name, body));

    return fn;
}

Stmt* Parser::parseIfStmt()
{
    Token tk = eat();
    expect(Lexer::OpenParen, "Expected opening parethesis");

    Expr* condition = parseExpr();

    Token lastToken = eat();
    if (lastToken.type != Lexer::ClosedParen)
    {
        std::cerr << SyntaxError("Expected closing parentheses, recieved " + lastToken.value, lastToken, context);
        exit(1);
    }

    std::vector<Stmt*> body = parseBody();

    IfStmtType* ifStmt = newNode<IfStmtType>(tk, condition, body);

    if (at().type == Lexer::Else)
    {
        eat();
        ifStmt->elseStmt = parseBody();
        ifStmt->hasElse = true;
    }

    return ifStmt;
}

VarDeclarationType* Parser::parseVarDeclaration(bool isConstant, bool tkEaten)
{
    Token tk = at();
    if (!tkEaten) eat();
    std::string ident = expect(Lexer::Identifier, "Expected identifier").value;

    bool hasType = false;
    Expr* type;

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
            std::cout << SyntaxError("Must assign value to constant variable", at(), context);
            exit(1);
        }

        if (!hasType)
            return newNode<VarDeclarationType>(tk, new UndefinedLiteralType(), ident);
        else
            return newNode<VarDeclarationType>(tk, new UndefinedLiteralType(), ident, type);
    }

    
    eat();
    if (!hasType)
        return newNode<VarDeclarationType>(tk, parseExpr(), ident);
    else
        return newNode<VarDeclarationType>(tk, parseExpr(), ident, type);
}

Expr* Parser::parseExpr()
{
    return parseAssignmentExpr();
}

Expr* Parser::parseAssignmentExpr()
{
    Expr* left = parseTernaryExpr();

    if (at().type == Lexer::Increment || at().type == Lexer::Decrement)
    {
        Token op = eat();
        return newNode<UnaryPostFixType>(op, op.value, left);
    }

    if (at().type == Lexer::Equals || at().type == Lexer::AssignmentOperator)
    {
        Token op = eat();
        Expr* value = parseExpr();
        return newNode<AssignmentExprType>(op, left, value, op.value);
    }

    return left;
}

Expr* Parser::parseTernaryExpr()
{
    Expr* cond = parseCallMemberExpr();

    if (at().type != Lexer::Ternary) return cond;

    Token tk = eat();

    Expr* cons = parseExpr();
    expect(Lexer::Colon, "Expected colon after ternary consequent");
    Expr* alt = parseExpr();

    return newNode<TernaryExprType>(tk, cond, cons, alt);
}

Expr* Parser::parseCallMemberExpr()
{
    Expr* member = parseMemberExpr();

    if (at().type == Lexer::OpenParen)
    {
        return parseCallexpr(member);
    } else return member;
}

Expr* Parser::parseMemberExpr()
{
    Expr* obj = parseLogicalExpr();

    while (at().type == Lexer::Dot || at().type == Lexer::OpenBracket)
    {
        Token op = eat();
        Expr* property;
        bool computed;
        std::string lastProp;

        if (op.type == Lexer::Dot)
        {
            computed = false;
            property = parsePrimaryExpr();

            if (property->kind != NodeType::Identifier)
            {
                std::cerr << SyntaxError("Cannot use dot operator without right hand side being an identifier", op, context);
                exit(1);
            }

            lastProp = static_cast<IdentifierType*>(property)->symbol;

        } else
        {
            computed = true;
            property = parseExpr();
            expect(Lexer::CloseBracket, "Expected closing bracket");

            if (property->kind == NodeType::StringLiteral)
            {
                lastProp = static_cast<StringLiteralType*>(property)->strValue;
            }
        }

        if (at().type == Lexer::Equals)
        {
            eat();
            Expr* value = parseExpr();
            obj = newNode<MemberAssignmentType>(op, obj, property, value, computed);
        } else
        {
            MemberExprType* member = newNode<MemberExprType>(op, obj, property, computed);
            member->lastProp = lastProp;
            obj = member;
        }
    }

    return obj;
}

Expr* Parser::parseLogicalExpr()
{
    Expr* left = parseEqualityExpr();

    while (at().type == Lexer::AndOperator || at().type == Lexer::OrOperator)
    {
        Token op = eat();
        Expr* right = parseEqualityExpr();
        left = newNode<BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

Expr* Parser::parseEqualityExpr()
{
    Expr* left = parseRelationalExpr();

    while (at().type == Lexer::DoubleEquals || at().type == Lexer::NotEquals)
    {
        Token op = eat();

        Expr* right = parseRelationalExpr();
        left = newNode<BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

Expr* Parser::parseRelationalExpr()
{
    Expr* left = parseObjectExpr();

    while (at().value == "<" || at().value == ">" || at().value == "<=" || at().value == ">=")
    {
        Token op = eat();

        Expr* right = parseExpr();

        left = newNode<BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

Expr* Parser::parseObjectExpr()
{
    if (at().type != Lexer::Openbrace)
    {
        return parseAdditiveExpr();
    }

    Token tk = eat();

    std::vector<PropertyLiteralType*> properties;

    while (notEOF() && at().type != Lexer::ClosedBrace)
    {
        Token key = eat();

        if (at().type == Lexer::Comma)
        {
            eat();
            properties.push_back(newNode<PropertyLiteralType>(key, key.value, new UndefinedLiteralType()));
            continue;
        } else if (at().type == Lexer::ClosedBrace)
        {
            properties.push_back(newNode<PropertyLiteralType>(key, key.value, new UndefinedLiteralType()));
            continue;
        }

        expect(Lexer::Colon, "Expected colon");

        Expr* value = parseExpr();

        properties.push_back(newNode<PropertyLiteralType>(key, key.value, value));

        if (at().type != Lexer::ClosedBrace)
        {
            expect(Lexer::Comma, "Expected comma");
        }
    }

    expect(Lexer::ClosedBrace, "Object literal missing closing bracket");

    return newNode<MapLiteralType>(tk, properties);
}

Expr* Parser::parseAdditiveExpr()
{
    Expr* left = parseMultiplicativeExpr();
    while (at().value == "+" || at().value == "-")
    {
        Token op = eat();
        Expr* right = parseMultiplicativeExpr();
        left = newNode<BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

Expr* Parser::parseMultiplicativeExpr()
{
    Expr* left = parseUnaryExpr();

    while (at().value == "*" || at().value == "/" || at().value == "%")
    {
        Token op = eat();
        Expr* right = parseUnaryExpr();
        left = newNode<BinaryExprType>(op, left, right, op.value);
    }

    return left;
}

Expr* Parser::parseUnaryExpr()
{
    if (at().type == Lexer::Bang ||
        at().type == Lexer::Increment || 
        at().type == Lexer::Decrement
    )
    {

        Token op = eat();
        Expr* argument = parseExpr();
        return newNode<UnaryPrefixType>(op, op.value, argument);
    }

    return parseArrowFunction();
}


Expr* Parser::parseCallexpr(Expr* caller)
{
    CallExprType* callExpr = newNode<CallExprType>(at(), caller, parseArgs());
    if (at().type == Lexer::OpenParen) return parseCallexpr(callExpr);
    
    if (at().type == Lexer::Dot || at().type == Lexer::OpenBracket)
    {
        return parseMemberChain(callExpr);
    }
    
    return callExpr;
}

Expr* Parser::parseArrowFunction()
{
    if (at().type != Lexer::Function)
    {
        return parseNewExpr();
    }

    Token tk = eat();

    std::vector<VarDeclarationType*> params = parseParams();

    expect(Lexer::Arrow, "Expected arrow after arrow function parameters");

    std::vector<Stmt*> body = parseBody();

    return newNode<ArrowFunctionType>(tk, params, body);
}

Expr* Parser::parseNewExpr()
{
    if (at().type != Lexer::New)
    {
        return parsePrimaryExpr();
    }

    Token tk = eat();

    Expr* constructor = parseMemberExpr();

    std::vector<Expr*> args;
    if (at().type == Lexer::OpenParen)
    {
        args = parseArgs();
    }

    return newNode<NewExprType>(tk, constructor, args);
}

Expr* Parser::parseMemberChain(Expr* expr)
{
    while (at().type == Lexer::Dot || at().type == Lexer::OpenBracket)
    {
        Token op = eat();
        Expr* property;
        bool computed;
        std::string lastProp;
        
        if (op.type == Lexer::Dot)
        {
            computed = false;
            property = parsePrimaryExpr();
            
            if (property->kind != NodeType::Identifier)
            {
                std::cerr << SyntaxError("Cannot use dot operator without right hand side being an identifier", op, context);
                exit(1);
            }
            
            lastProp = static_cast<IdentifierType*>(property)->symbol;
            
        } else
        {
            computed = true;
            property = parseExpr();
            expect(Lexer::CloseBracket, "Expected closing bracket");
            
            if (property->kind == NodeType::StringLiteral)
            {
                lastProp = static_cast<StringLiteralType*>(property)->strValue;
            }
        }
        
        if (at().type == Lexer::Equals)
        {
            eat();
            Expr* value = parseExpr();
            expr = newNode<MemberAssignmentType>(op, expr, property, value, computed);
        } else
        {
            MemberExprType* member = newNode<MemberExprType>(op, expr, property, computed);
            member->lastProp = lastProp;
            expr = member;
        }
        
        if (at().type == Lexer::OpenParen)
        {
            expr = parseCallexpr(expr);
        }
    }
    
    return expr;
}

Expr* Parser::parsePrimaryExpr()
{
    Token tk = at();
    if (tk.type == Lexer::OpenBracket)
    {
        return parseArrayExpr();
    }

    switch (tk.type)
    {
        case Lexer::Identifier:
        {
            Token token = eat();
            return newNode<IdentifierType>(token, token.value);
        }

        case Lexer::Number:
        {
            Token token = eat();
            return newNode<NumericLiteralType>(token, std::stod(token.value));
        }

        case Lexer::String:
        {
            Token token = eat();
            return newNode<StringLiteralType>(token, token.value);
        }

        case Lexer::OpenParen:
        {
            eat();
            Expr* value = parseExpr();
            expect(Lexer::ClosedParen, "Expected closing parentheses ");
            return value;
        }

        case Lexer::Bool:
            return newNode<BoolLiteralType>(at(), eat().value == "true");

        case Lexer::Undefined:
            return newNode<UndefinedLiteralType>(at());

        case Lexer::Null:
            eat();
            return new NullLiteralType();

        case Lexer::END:
            eat();
            return new UndefinedLiteralType();

        default:
            std::cout << SyntaxError("Unexpected token found while parsing: " + at().value, at(), context);
            exit(1);
    }
}

Expr* Parser::parseArrayExpr()
{
    if (at().type != Lexer::OpenBracket)
    {
        return parseCallMemberExpr();
    }

    Token tk = eat();

    std::vector<Expr*> items;

    if (at().type == Lexer::CloseBracket)
    {
        eat();
        return newNode<ArrayLiteralType>(tk, items);
    }
    
    items.push_back(parseExpr());

    while (at().type == Lexer::Comma)
    {
        eat();
        items.push_back(parseExpr());
    }

    expect(Lexer::CloseBracket, "Expected closing bracket");

    return newNode<ArrayLiteralType>(tk, items);
}

std::vector<Expr*> Parser::parseArgs()
{
    expect(Lexer::OpenParen, "Expected open parentheses");

    std::vector<Expr*> args; 
    if (at().type != Lexer::ClosedParen)
    {
        args = parseArgList();
    }

    expect(Lexer::ClosedParen, "Expected closing parentheses");
    
    return args;
}

std::vector<Expr*> Parser::parseArgList()
{
    std::vector<Expr*> args;

    args.push_back(parseAssignmentExpr());

    while (notEOF() && at().type == Lexer::Comma)
    {
        eat();
        args.push_back(parseAssignmentExpr());
    }

    return args;
}

VarDeclarationType* Parser::parseParam() {
    Token ident = expect(Lexer::Identifier, "Expected identifier");

    Expr* type = nullptr;
    Expr* value = nullptr;

    if (at().type == Lexer::Colon) {
        eat();
        type = parseExpr();
    }

    if (at().type == Lexer::Equals) {
        Token eq = eat();
        value = parseExpr();
        return newNode<VarDeclarationType>(eq, value, ident.value, type ? type : new UndefinedLiteralType());
    }

    return newNode<VarDeclarationType>(ident, new UndefinedLiteralType(), ident.value, type);
}

std::vector<VarDeclarationType*> Parser::parseParams() {
    expect(Lexer::OpenParen, "Expected parentheses before parameters");
    std::vector<VarDeclarationType*> params;

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

std::vector<Stmt*> Parser::parseBody(bool methods, std::string prbname)
{
    if (at().type == Lexer::Openbrace)
    {
        eat();
        std::vector<Stmt*> body;
        if (methods)
        {
            while (at().type != Lexer::ClosedBrace && at().type != Lexer::END)
            {
                if (at().type == Lexer::Identifier)
                {
                    std::string name = at().value;
                    if (at(1).type == Lexer::OpenParen)
                    {
                        Stmt* fn = parseFunctionDeclaration(true);

                        if (fn->kind == NodeType::FunctionDeclaration)
                        {
                            FunctionDeclarationType* func = static_cast<FunctionDeclarationType*>(fn);
                            
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
        std::vector<Stmt*> body;
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
        std::cerr << SyntaxError(err, prev, context);
        exit(1);
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