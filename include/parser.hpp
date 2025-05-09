#pragma once
#include "ast.hpp"
#include "lexer.hpp"
#include <iostream>
#include "utils/shift.hpp"


class Parser {
    public:
        ProgramType* produceAST(std::string& sourceCode) {
            tokens = Lexer::tokenize(sourceCode);
            ProgramType* program = new ProgramType();
    
            while (notEOF()) {
                program->body.push_back(parseStmt());
            }
    
            return program;
        }
    
    private:
        std::vector<Lexer::Token> tokens;
    
        bool notEOF() {
            return tokens[0].type != Lexer::END;
        }
    
        Stmt* parseStmt() {
            switch (at().type) {
                case Lexer::Probe:
                    return parseProbeDeclaration();
                case Lexer::Var:
                    return parseVarDeclaration();
                case Lexer::Const:
                    return parseVarDeclaration(true);
                case Lexer::Function:
                    return parseFunctionDeclaration();
                case Lexer::Semicolon:
                    eat();
                    return parseStmt();
                case Lexer::Module:
                    return parseModuleDeclaration();
                case Lexer::If:
                    return parseIfStmt();
                case Lexer::Import:
                    return parseImportStmt();
                case Lexer::Return:
                    return parseReturnStmt();
                case Lexer::Export:
                    return parseExportStmt();
                case Lexer::While:
                    return parseWhileStmt();
                case Lexer::Class:
                    return parseClassDeclaration();
                case Lexer::For:
                    return parseForStmt();
                default:
                    return parseExpr();
            }
        }

        Stmt* parseProbeDeclaration() {
            eat();
            std::string name = expect(Lexer::Identifier, "Expected identifier").value;

            if (at().type == Lexer::Extends) {
                eat();
                Expr* extends = parseExpr();
                std::vector<Stmt*> body = parseBody();

                return new ProbeDeclarationType(name, body, extends);
            }

            std::vector<Stmt*> body = parseBody();

            ProbeDeclarationType* prb = new ProbeDeclarationType(name, body);

            return prb;
        }
        Stmt* parseForStmt() {
            eat();
            expect(Lexer::OpenParen, "Expected '(' after 'for'");
        
            std::vector<Stmt*> decl;
            if (at().type == Lexer::OpenParen) {
                eat();
                while (at().type != Lexer::ClosedParen && at().type != Lexer::END) {
                    decl.push_back(parseStmt());
                    if (at().type == Lexer::Comma) eat();
                    else break;
                }
                expect(Lexer::ClosedParen, "Expected ')' after initializer block");
            } else if (at().type != Lexer::Comma) {
                decl.push_back(parseStmt());
            }
            expect(Lexer::Comma, "Expected ',' after initializer in for loop");
        
            std::vector<Expr*> cond;
            if (at().type == Lexer::OpenParen) {
                eat();
                while (at().type != Lexer::ClosedParen && at().type != Lexer::END) {
                    cond.push_back(parseExpr());
                    if (at().type == Lexer::Comma) eat();
                    else break;
                }
                expect(Lexer::ClosedParen, "Expected ')' after condition block");
            } else if (at().type != Lexer::Comma) {
                cond.push_back(parseExpr());
            }
            expect(Lexer::Comma, "Expected ',' after condition in for loop");
        
            std::vector<Expr*> update;
            if (at().type == Lexer::OpenParen) {
                eat();
                while (at().type != Lexer::ClosedParen && at().type != Lexer::END) {
                    update.push_back(parseExpr());
                    if (at().type == Lexer::Comma) eat();
                    else break;
                }
                expect(Lexer::ClosedParen, "Expected ')' after update block");
            } else if (at().type != Lexer::ClosedParen) {
                update.push_back(parseExpr());
            }
        
            expect(Lexer::ClosedParen, "Expected ')' after update in for loop");
        
            std::vector<Stmt*> body = parseBody();
            return new ForStmtType(decl, cond, update, body);
        }
        

        Stmt* parseReturnStmt() {
            eat();

            return new ReturnStmtType(parseStmt());
        }

        Stmt* parseClassDeclaration() {
            eat();
            std::string name = expect(Lexer::Identifier, "Expected identifier").value;

            if (at().type == Lexer::Extends) {
                eat();
                Expr* extends = parseExpr();
                std::vector<Stmt*> body = parseBody();

                return new ClassDefinitionType(name, body, extends);
            }

            std::vector<Stmt*> body = parseBody();

            return new ClassDefinitionType(name, body);
        }

        Stmt* parseModuleDeclaration() {
            eat();
            expect(Lexer::Identifier, "Expected Identifier after module declaration");

            return new UndefinedLiteralType();
        }

        Stmt* parseWhileStmt() {
            eat();
            expect(Lexer::OpenParen, "Expected open parentheses after while keyword");

            Expr* condition = parseExpr();

            expect(Lexer::ClosedParen, "Expected closing parentheses after while condition");

            std::vector<Stmt*> body = parseBody();
            return new WhileStmtType(condition, body);
        }

        Stmt* parseImportStmt() {   
            eat();
            std::string name = at().value;
            if (at(1).type == Lexer::Dot) {
                Expr* module = parseExpr();

                if (at().type == Lexer::As) {
                    eat();
                    std::string ident = expect(Lexer::Identifier, "Expected identifier after as keyword").value;
                    ImportStmtType* importstmt = new ImportStmtType(name, module, ident);
                    return importstmt;
                }

                ImportStmtType* importstmt = new ImportStmtType(name, module);
                return importstmt;
            } else eat();

            if (at().type == Lexer::As) {
                eat();
                std::string ident = expect(Lexer::Identifier, "Expected identifier after as keyword").value;
                ImportStmtType* importstmt = new ImportStmtType(name, ident);
                return importstmt;
            }

            ImportStmtType* importstmt = new ImportStmtType(name);

            return importstmt;
        }

        Stmt* parseExportStmt() {
            eat();
            Stmt* value = parseStmt();

            ExportStmtType* exportstmt = new ExportStmtType(value);

            return exportstmt;
        }

        Stmt* parseFunctionDeclaration() {
            eat();
            std::string name = expect(Lexer::Identifier, "Expected identifier").value;

            std::vector<Expr*> args = parseArgs();
        
            std::vector<std::string> params;
            
            for (Expr* arg : args) {
                if (arg->kind != NodeType::Identifier) {
                    std::cerr << "Expected parameter to be of type identifier";
                    exit(1);
                }

                params.push_back(static_cast<IdentifierType*>(arg)->symbol);
            }

            std::vector<Stmt*> body = parseBody();


            FunctionDeclarationType* fn = new FunctionDeclarationType(params, name, body);

            return fn;
        }

        Stmt* parseIfStmt() {
            eat();
            expect(Lexer::OpenParen, "Expected opening parethesis");

            Expr* condition = parseExpr();

            Lexer::Token lastToken = eat();
            if (lastToken.type != Lexer::ClosedParen) {
                std::cerr << "Expected closing parentheses, recieved " << lastToken.value;
                exit(1);
            }

            
            std::vector<Stmt*> body = parseBody();

            IfStmtType* ifStmt = new IfStmtType(condition, body);

            if (at().type == Lexer::Else) {
                eat();
                ifStmt->elseStmt = parseBody();
                ifStmt->hasElse = true;
            }

            return ifStmt;
        }

        VarDeclarationType* parseVarDeclaration(bool isConstant = false) {
            eat();
            std::string ident = expect(Lexer::Identifier, "Expected identifier, recieved: ").value;

            if (at().type != Lexer::Equals) {
                if (isConstant) {
                    std::cout << "Must assign value to constant variable";
                    exit(1);
                }
                return new VarDeclarationType(new UndefinedLiteralType(), ident);
            }
            
            expect(Lexer::Equals, "Expected equals token in variable declaration, revieved: ");
            VarDeclarationType* declaration = new VarDeclarationType(parseExpr(), ident);
            
            return declaration;
        }

        Expr* parseExpr() {
            return parseAssignmentExpr();
        }

        Expr* parseAssignmentExpr() {
            Expr* left = parseLogicalExpr();
        
            if (at().type == Lexer::Increment || at().type == Lexer::Decrement) {
                std::string op = eat().value;
                return new UnaryPostFixType(op, left);
            }
        
            if (at().type == Lexer::Equals || at().type == Lexer::AssignmentOperator) {
                std::string op = eat().value;
                Expr* value = parseAssignmentExpr();
                return new AssignmentExprType(left, value, op);
            }
        
            return left;
        }
        

        Expr* parseLogicalExpr() {
            Expr* left = parseEqualityExpr();

            while (at().type == Lexer::AndOperator || at().type == Lexer::OrOperator) {
                std::string op = eat().value;
                Expr* right = parseEqualityExpr();
                left = new BinaryExprType(left, right, op);
            }

            return left;
        }

        Expr* parseEqualityExpr() {
            Expr* left = parseRelationalExpr();

            while (at().type == Lexer::DoubleEquals || at().type == Lexer::NotEquals) {
                std::string op = eat().value;

                Expr* right = parseRelationalExpr();
                left = new BinaryExprType(left, right, op);
            }

            return left;
        }

        Expr* parseRelationalExpr() {
            Expr* left = parseObjectExpr();

            while (at().value == "<" || at().value == ">" || at().value == "<=" || at().value == ">=") {
                std::string op = eat().value;

                Expr* right = parseObjectExpr();

                left = new BinaryExprType(left, right, op);
            }

            return left;
        }

        Expr* parseObjectExpr() {
            if (at().type != Lexer::Openbrace) {
                return parseAdditiveExpr();
            }

            eat();

            std::vector<PropertyLiteralType*> properties;

            while (notEOF() && at().type != Lexer::ClosedBrace) {
                std::string key = expect(Lexer::Identifier, "Object literal key expected").value;

                if (at().type == Lexer::Comma) {
                    eat();
                    properties.push_back(new PropertyLiteralType(key, new UndefinedLiteralType()));
                    continue;
                } else if (at().type == Lexer::ClosedBrace) {
                    properties.push_back(new PropertyLiteralType(key, new UndefinedLiteralType()));
                    continue;
                }

                expect(Lexer::Colon, "Expected colon");

                Expr* value = parseExpr();


                properties.push_back(new PropertyLiteralType(key, value));

                if (at().type != Lexer::ClosedBrace) {
                    expect(Lexer::Comma, "Expected comma");
                }
            }

            expect(Lexer::ClosedBrace, "Object literal missing closing bracket");

            return new ObjectLiteralType(properties);
        }
    
        Expr* parseAdditiveExpr() {
            Expr* left = parseMultiplicativeExpr();
            while (at().value == "+" || at().value == "-") {
                std::string op = eat().value;
                Expr* right = parseMultiplicativeExpr();
                left = new BinaryExprType(left, right, op);
            }
    
            return left;
        }

        Expr* parseMultiplicativeExpr() {

            Expr* left = parseUnaryExpr();

            while (at().value == "*" || at().value == "/" || at().value == "%") {
                std::string op = eat().value;
                Expr* right = parseUnaryExpr();
                left = new BinaryExprType(left, right, op);
            }
    
            return left;
        }

        Expr* parseUnaryExpr() {
            if (at().type == Lexer::Bang ||
                at().type == Lexer::Increment || 
                at().type == Lexer::Decrement
            ) {
        
                std::string op = eat().value;
                Expr* argument = parseUnaryExpr();
                return new UnaryPrefixType(op, argument);
            }
        
            return parseArrowFunction();
        }
    

        Expr* parseCallMemberExpr() {
            Expr* member = parseMemberExpr();

            if (at().type == Lexer::OpenParen) {
                return parseCallexpr(member);
            } else return member;
        }

        Expr* parseCallexpr(Expr* caller) {
            std::vector<Expr*> args = parseArgs();
            return new CallExprType(caller, args);
        }

        Expr* parseArrowFunction() {
            if (at().type != Lexer::OpenParen && at().type != Lexer::Identifier) {
                return parseNewExpr();
            }

            std::vector<std::string> params;
            int i = 0;
            bool isArrow = false;

            if (at().type == Lexer::Identifier && at(1).type == Lexer::Arrow) {
                params.push_back(eat().value);
                isArrow = true;
            } else if (at().type == Lexer::OpenParen) {
                i++;

                while (i < tokens.size() && tokens[i].type != Lexer::ClosedParen) {
                    if (tokens[i].type == Lexer::Identifier || tokens[i].type == Lexer::Comma) {
                        i++;
                    }
                }

                if (i < tokens.size() && tokens[i].type == Lexer::ClosedParen) {
                    i++;

                    if (i < tokens.size() && tokens[i].type == Lexer::Arrow) {
                        isArrow = true;
                    }
                }
            }

            if (!isArrow) {
                return parseNewExpr();
            }

            if (at().type == Lexer::OpenParen) {
                eat();

                if (at().type != Lexer::ClosedParen) {
                    params.push_back(expect(Lexer::Identifier, "Expected identifier in arrow function parameters").value);

                    while (at().type == Lexer::Comma) {
                        eat();
                        params.push_back(expect(Lexer::Identifier, "Trailing comma not allowed in arrow function parameters").value);
                    }
                }

                expect(Lexer::ClosedParen, "Expected closing parentheses after arrow function parameters");
            }

            expect(Lexer::Arrow, "Expected arrow after arrow function parameters");

            std::vector<Stmt*> body = parseBody();

            return new ArrowFunctionType(params, body);
        }

        Expr* parseNewExpr() {
            if (at().type != Lexer::New) {
                return parseCallMemberExpr();
            }

            eat();

            Expr* constructor = parseMemberExpr();

            std::vector<Expr*> args;
            if (at().type == Lexer::OpenParen) {
                args = parseArgs();
            }

            return new NewExprType(constructor, args);
        }
        
        Expr* parseMemberExpr() {
            Expr* obj = parsePrimaryExpr();
        
            while (at().type == Lexer::Dot || at().type == Lexer::OpenBracket) {
                Lexer::Token op = eat();
                Expr* property;
                bool computed;
                std::string lastProp;
        
                if (op.type == Lexer::Dot) {
                    computed = false;
                    property = parsePrimaryExpr();
        
                    if (property->kind != NodeType::Identifier) {
                        std::cerr << "Cannot use dot operator without right hand side being an identifier";
                        exit(1);
                    }
        
                    lastProp = static_cast<IdentifierType*>(property)->symbol;
        
                } else {
                    computed = true;
                    property = parseExpr();
                    expect(Lexer::CloseBracket, "Expected closing bracket");
        
                    if (property->kind == NodeType::StringLiteral) {
                        lastProp = static_cast<StringLiteralType*>(property)->strValue;
                    }
                }
        
                if (at().type == Lexer::Equals) {
                    eat();
                    Expr* value = parseExpr();
                    obj = new MemberAssignmentType(obj, property, value, computed);
                } else {
                    MemberExprType* member = new MemberExprType(obj, property, computed);
                    member->lastProp = lastProp;
                    obj = member;
                }
            }
        
            return obj;
        }
        
    
        Expr* parsePrimaryExpr() {
            Lexer::TokenType tk = at().type;
            if (tk == Lexer::OpenBracket) {
                return parseArrayExpr();
            }

            switch (tk) {
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
                    eat();
                    return value;
                }

                case Lexer::Null:
                    eat();
                    return new NullLiteralType();

                case Lexer::END:
                    eat();
                    return new NullLiteralType();
    
                default:
                    std::cout << "Unexpected token found while parsing: ";
                    std::cout << "\"" << at().value << "\": " << at().type << std::endl;
                    exit(1);
            }
        }
        
        Expr* parseArrayExpr() {
            if (at().type != Lexer::OpenBracket) {
                return parseCallMemberExpr();
            }

            eat();

            std::vector<Expr*> items;

            if (at().type == Lexer::CloseBracket) {
                eat();
                return new ArrayLiteralType(items);
            }
            
            items.push_back(parseExpr());

            while (at().type == Lexer::Comma) {
                eat();
                items.push_back(parseExpr());
            }

            expect(Lexer::CloseBracket, "Expected closing bracket");

            return new ArrayLiteralType(items);
        }

        std::vector<Expr*> parseArgs() {
            expect(Lexer::OpenParen, "Expected open parentheses");

            std::vector<Expr*> args; 
            if (at().type != Lexer::ClosedParen) {
                args = parseArgList();
            };

            expect(Lexer::ClosedParen, "Expected closing parentheses");
            
            return args;
        }

        std::vector<Expr*> parseArgList() {
            std::vector<Expr*> args;

            args.push_back(parseAssignmentExpr());

            while (notEOF() && at().type == Lexer::Comma) {
                eat();
                args.push_back(parseAssignmentExpr());
            }

            return args;
        }
    
        Lexer::Token at(int index = 0) {
            return tokens[index];
        }

        Lexer::Token next() {
            return tokens[1];
        }
    
        Lexer::Token eat() {
            Lexer::Token prev = shift(tokens);
            return prev;
        }

        Lexer::Token expect(Lexer:: TokenType type, std::string err) {
            Lexer::Token prev = shift(tokens);
            if (prev.type != type) {
                std::cerr << err;
                exit(1);
            }

            return prev;
        }

        std::vector<Stmt*> parseBody() {
            if (at().type == Lexer::Openbrace) {
                eat();
                std::vector<Stmt*> body;
                while (at().type != Lexer::ClosedBrace && at().type != Lexer::END) {
                    body.push_back(parseStmt());
                }

                expect(Lexer::ClosedBrace, "Expected closing brace");
                return body;
            } else {
                std::vector<Stmt*> body;
                body.push_back(parseStmt());
                return body;
            }
        }
};