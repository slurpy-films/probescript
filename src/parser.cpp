#include "parser.hpp"

ProgramType* Parser::produceAST(std::string& sourceCode, Context* ctx)
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
            eat();
            stmt = new BreakStmtType();
            break;
        case Lexer::Continue:
            eat();
            stmt = new ContinueStmtType();
            break;
        case Lexer::Try:
            stmt = parseTryStmt();
            break;
        case Lexer::Semicolon:
            eat();
            stmt = new UndefinedLiteralType();
            break;
        default:
            stmt = parseExpr();
    }

    if (at().type == Lexer::Semicolon) eat();
    return stmt;
}


Stmt* Parser::parseProbeDeclaration()
{
    eat();
    std::string name = expect(Lexer::Identifier, "Expected identifier").value;

    if (at().type == Lexer::Extends)
    {
        eat();
        Expr* extends = parseExpr();
        std::vector<Stmt*> body = parseBody(true);

        return new ProbeDeclarationType(name, body, extends);
    }

    std::vector<Stmt*> body = parseBody(true, name);

    ProbeDeclarationType* prb = new ProbeDeclarationType(name, body);

    return prb;
}

Stmt* Parser::parseTryStmt()
{
    eat();
    std::vector<Stmt*> body = parseBody();

    expect(Lexer::Catch, "Expected catch after try body");
    std::vector<VarDeclarationType*> params = parseParams();

    std::vector<Stmt*> catchBody = parseBody();

    return new TryStmtType(body, new FunctionDeclarationType(params, "catch", catchBody));
}

Stmt* Parser::parseForStmt()
{
    eat();
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
    return new ForStmtType(decl, cond, update, body);
}

Stmt* Parser::parseThrowStmt()
{
    eat();
    return new ThrowStmtType(parseExpr());
}

Stmt* Parser::parseReturnStmt()
{
    eat();
    return new ReturnStmtType(parseExpr());
}

Stmt* Parser::parseClassDeclaration()
{
    eat();
    std::string name = expect(Lexer::Identifier, "Expected identifier").value;

    if (at().type == Lexer::Extends)
    {
        eat();
        Expr* extends = parseExpr();
        std::vector<Stmt*> body = parseBody(true);

        return new ClassDefinitionType(name, body, extends);
    }

    std::vector<Stmt*> body = parseBody(true);

    return new ClassDefinitionType(name, body);
}

Stmt* Parser::parseModuleDeclaration()
{
    eat();
    expect(Lexer::Identifier, "Expected Identifier after module declaration");

    return new UndefinedLiteralType();
}

Stmt* Parser::parseWhileStmt()
{
    eat();
    expect(Lexer::OpenParen, "Expected open parentheses after while keyword");

    Expr* condition = parseExpr();

    expect(Lexer::ClosedParen, "Expected closing parentheses after while condition");

    std::vector<Stmt*> body = parseBody();
    return new WhileStmtType(condition, body);
}

Stmt* Parser::parseImportStmt()
{   
    eat();
    std::string name = at().value;
    if (at(1).type == Lexer::Dot)
    {
        Expr* module = parseExpr();

        if (at().type == Lexer::As)
        {
            eat();
            std::string ident = expect(Lexer::Identifier, "Expected identifier after as keyword").value;
            ImportStmtType* importstmt = new ImportStmtType(name, module, ident);
            return importstmt;
        }

        ImportStmtType* importstmt = new ImportStmtType(name, module);
        return importstmt;
    } else eat();

    if (at().type == Lexer::As)
    {
        eat();
        std::string ident = expect(Lexer::Identifier, "Expected identifier after as keyword").value;
        ImportStmtType* importstmt = new ImportStmtType(name, ident);
        return importstmt;
    }

    ImportStmtType* importstmt = new ImportStmtType(name);

    return importstmt;
}

Stmt* Parser::parseExportStmt()
{
    eat();
    Stmt* value = parseStmt();

    ExportStmtType* exportstmt = new ExportStmtType(value);

    return exportstmt;
}

Stmt* Parser::parseFunctionDeclaration(bool tkEaten)
{
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


    FunctionDeclarationType* fn = (type ? new FunctionDeclarationType(params, name, body, type) : new FunctionDeclarationType(params, name, body));

    return fn;
}

Stmt* Parser::parseIfStmt()
{
    eat();
    expect(Lexer::OpenParen, "Expected opening parethesis");

    Expr* condition = parseExpr();

    Lexer::Token lastToken = eat();
    if (lastToken.type != Lexer::ClosedParen)
    {
        std::cerr << SyntaxError("Expected closing parentheses, recieved " + lastToken.value, lastToken, context);
        exit(1);
    }

    
    std::vector<Stmt*> body = parseBody();

    IfStmtType* ifStmt = new IfStmtType(condition, body);

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
            return new VarDeclarationType(new UndefinedLiteralType(), ident);
        else
            return new VarDeclarationType(new UndefinedLiteralType(), ident, type);
    }

    
    eat();
    if (!hasType)
        return new VarDeclarationType(parseExpr(), ident);
    else
        return new VarDeclarationType(parseExpr(), ident, type);
}

Expr* Parser::parseExpr()
{
    return parseAssignmentExpr();
}

Expr* Parser::parseAssignmentExpr()
{
    Expr* left = parseCallMemberExpr();

    if (at().type == Lexer::Increment || at().type == Lexer::Decrement)
    {
        std::string op = eat().value;
        return new UnaryPostFixType(op, left);
    }

    if (at().type == Lexer::Equals || at().type == Lexer::AssignmentOperator)
    {
        std::string op = eat().value;
        Expr* value = parseExpr();
        return new AssignmentExprType(left, value, op);
    }

    return left;
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
        Lexer::Token op = eat();
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
            obj = new MemberAssignmentType(obj, property, value, computed);
        } else
        {
            MemberExprType* member = new MemberExprType(obj, property, computed);
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
        std::string op = eat().value;
        Expr* right = parseEqualityExpr();
        left = new BinaryExprType(left, right, op);
    }

    return left;
}

Expr* Parser::parseEqualityExpr()
{
    Expr* left = parseRelationalExpr();

    while (at().type == Lexer::DoubleEquals || at().type == Lexer::NotEquals)
    {
        std::string op = eat().value;

        Expr* right = parseRelationalExpr();
        left = new BinaryExprType(left, right, op);
    }

    return left;
}

Expr* Parser::parseRelationalExpr()
{
    Expr* left = parseObjectExpr();

    while (at().value == "<" || at().value == ">" || at().value == "<=" || at().value == ">=")
    {
        std::string op = eat().value;

        Expr* right = parseObjectExpr();

        left = new BinaryExprType(left, right, op);
    }

    return left;
}

Expr* Parser::parseObjectExpr()
{
    if (at().type != Lexer::Openbrace)
    {
        return parseAdditiveExpr();
    }

    eat();

    std::vector<PropertyLiteralType*> properties;

    while (notEOF() && at().type != Lexer::ClosedBrace)
    {
        std::string key = eat().value;

        if (at().type == Lexer::Comma)
        {
            eat();
            properties.push_back(new PropertyLiteralType(key, new UndefinedLiteralType()));
            continue;
        } else if (at().type == Lexer::ClosedBrace)
        {
            properties.push_back(new PropertyLiteralType(key, new UndefinedLiteralType()));
            continue;
        }

        expect(Lexer::Colon, "Expected colon");

        Expr* value = parseExpr();

        properties.push_back(new PropertyLiteralType(key, value));

        if (at().type != Lexer::ClosedBrace)
        {
            expect(Lexer::Comma, "Expected comma");
        }
    }

    expect(Lexer::ClosedBrace, "Object literal missing closing bracket");

    return new MapLiteralType(properties);
}

Expr* Parser::parseAdditiveExpr()
{
    Expr* left = parseMultiplicativeExpr();
    while (at().value == "+" || at().value == "-")
    {
        std::string op = eat().value;
        Expr* right = parseMultiplicativeExpr();
        left = new BinaryExprType(left, right, op);
    }

    return left;
}

Expr* Parser::parseMultiplicativeExpr()
{
    Expr* left = parseUnaryExpr();

    while (at().value == "*" || at().value == "/" || at().value == "%")
    {
        std::string op = eat().value;
        Expr* right = parseUnaryExpr();
        left = new BinaryExprType(left, right, op);
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

        std::string op = eat().value;
        Expr* argument = parseExpr();
        return new UnaryPrefixType(op, argument);
    }

    return parseArrowFunction();
}


Expr* Parser::parseCallexpr(Expr* caller)
{
    CallExprType* callExpr = new CallExprType(caller, parseArgs());
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

    eat();

    std::vector<VarDeclarationType*> params = parseParams();

    expect(Lexer::Arrow, "Expected arrow after arrow function parameters");

    std::vector<Stmt*> body = parseBody();

    return new ArrowFunctionType(params, body);
}

Expr* Parser::parseNewExpr()
{
    if (at().type != Lexer::New)
    {
        return parsePrimaryExpr();
    }

    eat();

    Expr* constructor = parseMemberExpr();

    std::vector<Expr*> args;
    if (at().type == Lexer::OpenParen)
    {
        args = parseArgs();
    }

    return new NewExprType(constructor, args);
}

Expr* Parser::parseMemberChain(Expr* expr)
{
    while (at().type == Lexer::Dot || at().type == Lexer::OpenBracket)
    {
        Lexer::Token op = eat();
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
            expr = new MemberAssignmentType(expr, property, value, computed);
        } else
        {
            MemberExprType* member = new MemberExprType(expr, property, computed);
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
    Lexer::TokenType tk = at().type;
    if (tk == Lexer::OpenBracket)
    {
        return parseArrayExpr();
    }

    switch (tk)
    {
        case Lexer::Identifier:
            return new IdentifierType(eat().value);

        case Lexer::Number:
            return new NumericLiteralType(std::stod(eat().value));

        case Lexer::String:
            return new StringLiteralType(eat().value);

        case Lexer::OpenParen: {
            eat();
            Expr* value = parseExpr();
            expect(Lexer::ClosedParen, "Expected closing parentheses ");
            return value;
        }

        case Lexer::Bool:
            return new BoolLiteralType(eat().value == "true");

        case Lexer::Undefined:
            eat();
            return new UndefinedLiteralType();

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

    eat();

    std::vector<Expr*> items;

    if (at().type == Lexer::CloseBracket)
    {
        eat();
        return new ArrayLiteralType(items);
    }
    
    items.push_back(parseExpr());

    while (at().type == Lexer::Comma)
    {
        eat();
        items.push_back(parseExpr());
    }

    expect(Lexer::CloseBracket, "Expected closing bracket");

    return new ArrayLiteralType(items);
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

std::vector<VarDeclarationType*> Parser::parseParams()
{
    expect(Lexer::OpenParen, "Expected parenteses before parameters");
    std::vector<VarDeclarationType*> params;
    if (at().type == Lexer::ClosedParen)
    {
        eat();
        return params;
    }

    std::string ident = expect(Lexer::Identifier, "Expected identifier").value;

    if (at().type == Lexer::Colon)
    {
        eat();
        Expr* type = parseExpr();

        if (at().type == Lexer::Equals)
        {
            eat();
            Expr* value = parseExpr();
            params.push_back(new VarDeclarationType(value, ident, type));
        } else params.push_back(new VarDeclarationType(new UndefinedLiteralType(), ident, type));
    } else
    {
        if (at().type == Lexer::Equals)
        {
            eat();
            Expr* value = parseExpr();
            params.push_back(new VarDeclarationType(value, ident));
        } else params.push_back(new VarDeclarationType(new UndefinedLiteralType(), ident));
    }

    while (at().type == Lexer::Comma)
    {
        eat();

        std::string ident = expect(Lexer::Identifier, "Expected identifier").value;

        if (at().type == Lexer::Colon)
        {
            eat();
            Expr* type = parseExpr();

            if (at().type == Lexer::Equals)
            {
                eat();
                Expr* value = parseExpr();
                params.push_back(new VarDeclarationType(value, ident, type));
            }
                else params.push_back(new VarDeclarationType(new UndefinedLiteralType(), ident));
        } else
        {
            if (at().type == Lexer::Equals)
            {
                eat();
                Expr* value = parseExpr();
                params.push_back(new VarDeclarationType(value, ident));
            }
                else params.push_back(new VarDeclarationType(new UndefinedLiteralType(), ident));
        }
    }

    expect(Lexer::ClosedParen, "Expected closing parenteses after parameters");
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