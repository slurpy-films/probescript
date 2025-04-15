#pragma once
#include "ast.hpp"
#include "lexer.hpp"
#include <iostream>
#include "utils/shift.hpp"

using namespace std;

class Parser {
    public:
        ProgramType* produceAST(string& sourceCode) {
            tokens = Lexer::tokenize(sourceCode);
            ProgramType* program = new ProgramType();
    
            while (notEOF()) {
                program->body.push_back(parseStmt());
            }
    
            return program;
        }
    
    private:
        vector<Lexer::Token> tokens;
    
        bool notEOF() {
            return tokens[0].type != Lexer::END;
        }
    
        Stmt* parseStmt() {
            switch (at().type) {
                case Lexer::Var:
                    return parseVarDeclaration();
                case Lexer::Const:
                    return parseVarDeclaration(true);
                case Lexer::Function:
                    return parseFunctionDeclaration();
                default:
                    return parseExpr();
            }
        }

        Stmt* parseFunctionDeclaration() {
            eat();
            string name = expect(Lexer::Identifier, "Expected identifier").value;

            vector<Expr*> args = parseArgs();
        
            vector<string> params;
            
            for (Expr* arg : args) {
                if (arg->kind != NodeType::Identifier) {
                    cerr << "Expected parameter to be of type string";
                    exit(1);
                }

                params.push_back(static_cast<IdentifierType*>(arg)->symbol);
            }

            expect(Lexer::Openbrace, "Expected function declaration expression");

            vector<Stmt*> body;

            while (at().type != Lexer::END && at().type != Lexer::ClosedBrace) {
                body.push_back(parseStmt());
            }

            expect(Lexer::ClosedBrace, "Expected closing brace");

            FunctionDeclarationType* fn = new FunctionDeclarationType(params, name, body);

            return fn;
        }

        VarDecalarationType* parseVarDeclaration(bool isConstant = false) {
            eat();
            string ident = expect(Lexer::Identifier, "Expected identifier, recieved: ").value;

            if (at().type == Lexer::Semicolon) {
                eat();
                if (isConstant) {
                    cout << "Must assign value to constant variable";
                }
                return new VarDecalarationType(nullptr, ident);
            }
            
            expect(Lexer::Equals, "Expected equals token in variable declaration, revieved: ");
            VarDecalarationType* declaration = new VarDecalarationType(parseExpr(), ident);
            
            return declaration;
        }

        Expr* parseExpr() {
            return parseAssignmentExpr();
        }

        Expr* parseAssignmentExpr() {
            Expr* left = parseObjectExpr();

            if (at().type == Lexer::Equals) {
                eat();
                Expr* value = parseAssignmentExpr();
                return new AssignmentExprType(left, value);
            }
            
            return left;
        }

        Expr* parseObjectExpr() {
            if (at().type != Lexer::Openbrace) {
                return parseAdditiveExpr();
            }

            eat();

            vector<PropertyLiteralType*> properties;

            while (notEOF() && at().type != Lexer::ClosedBrace) {
                string key = expect(Lexer::Identifier, "Object literal key expected").value;

                if (at().type == Lexer::Comma) {
                    eat();
                    properties.push_back(new PropertyLiteralType(key, new NullLiteralType()));
                    continue;
                } else if (at().type == Lexer::ClosedBrace) {
                    properties.push_back(new PropertyLiteralType(key, new NullLiteralType()));
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
                string op = eat().value;
                Expr* right = parseMultiplicativeExpr();
                left = new BinaryExprType(left, right, op);
            }
    
            return left;
        }

        Expr* parseMultiplicativeExpr() {

            Expr* left = parseCallMemberExpr();

            while (at().value == "*" || at().value == "/" || at().value == "%") {
                string op = eat().value;
                Expr* right = parseCallMemberExpr();
                left = new BinaryExprType(left, right, op);
            }
    
            return left;
        }

        Expr* parseCallMemberExpr() {
            Expr* member = parseMemberExpr();

            if (at().type == Lexer::OpenParen) {
                return parseCallexpr(member);
            } else return member;
        }

        Expr* parseCallexpr(Expr* caller) {
            vector<Expr*> args = parseArgs();
            return new CallExprType(caller, args);
        }
        

        vector<Expr*> parseArgs() {
            expect(Lexer::OpenParen, "Expected open parenthesis");

            vector<Expr*> args; 
            if (at().type != Lexer::ClosedParen) {
                args = parseArgList();
            };

            expect(Lexer::ClosedParen, "Expected closing parenthesis");
            
            return args;
        }

        vector<Expr*> parseArgList() {
            vector<Expr*> args = {};

            args.push_back(parseAssignmentExpr());

            while (notEOF() && at().type == Lexer::Comma) {
                eat();
                args.push_back(parseAssignmentExpr());
            }

            return args;
        }

        Expr* parseMemberExpr() {
            Expr* obj = parsePrimaryExpr();

            while (at().type == Lexer::Dot || at().type == Lexer::OpenBracket) {
                Lexer::Token op = eat();
                Expr* property;
                bool computed;

                if (op.type == Lexer::Dot) {
                    computed = false;
                    property = parsePrimaryExpr();

                    if (property->kind != NodeType::Identifier) {
                        cerr << "Cannot use dot operator without right hand side being an identifier";
                        exit(1);
                    }
                } else {
                    computed = true;
                    property = parseExpr();

                    expect(Lexer::CloseBracket, "Expected closing bracket");
                }

                obj = new MemberExprType(obj, property, computed);

            }

            return obj;
            
        }
    
        Expr* parsePrimaryExpr() {
            Lexer::TokenType tk = at().type;
    
            switch (tk) {
                case Lexer::Identifier:
                    return new IdentifierType(eat().value);
    
                case Lexer::Number:
                    return new NumericLiteralType(stod(eat().value));

                case Lexer::OpenParen: {
                    eat();
                    Expr* value = parseExpr();
                    expect(Lexer::ClosedParen, "Expected closing parenthesis ");
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
                    cout << "Unexpected token found while parsing: ";
                    cout << "\"" << at().value << "\": " << at().type << endl;
                    for (int i = 0; i < tokens.size(); ++i) {
                        cout << tokens[i].value << endl;
                    }
                    exit(1);
            }
        }
    
        Lexer::Token at() {
            return tokens[0];
        }

        Lexer::Token next() {
            return tokens[1];
        }
    
        Lexer::Token eat() {
            Lexer::Token prev = shift(tokens);
            return prev;
        }

        Lexer::Token expect(Lexer:: TokenType type, string err) {
            Lexer::Token prev = shift(tokens);
            if (prev.type != type) {
                cerr << err;
                exit(1);
            }

            return prev;
        }
};