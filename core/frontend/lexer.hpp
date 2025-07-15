#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "utils.hpp"
#include "context.hpp"

namespace Probescript::Lexer
{

enum TokenType
{
    Probe,
    Number,
    Identifier,
    Equals,
    OpenParen,
    ClosedParen,
    BinaryOperator,
    Var,
    END,
    Null,
    Semicolon,
    Const,
    Comma,
    Colon,
    OpenBrace,
    ClosedBrace,
    OpenBracket,
    CloseBracket,
    Dot,
    Function,
    String,
    If,
    Else,
    AndOperator,
    OrOperator,
    DoubleEquals,
    NotEquals,
    Import,
    Export,
    Module,
    While,
    AssignmentOperator,
    Class,
    New,
    Return,
    Extends,
    As,
    For,
    Increment,
    Decrement,
    Bang,
    Arrow,
    Break,
    Continue,
    Throw,
    Try,
    Catch,
    Undefined,
    Bool,
    Ternary,
    LessThan,
    GreaterThan,
    Async,
    Await,
};

struct Token
{
    std::string value;
    TokenType type;

    int line = 0;
    int col = 0;
    std::string file;
    std::shared_ptr<Context> ctx = std::make_shared<Context>();
};

inline Token token(const std::string& value, const TokenType type, const std::pair<int, int> pos)
{
    int line = pos.first;
    int col = pos.second;
    
    return { value, type, line, col };
}

inline bool isAlpha(const std::string& str)
{
    return (str[0] >= 'a' && str[0] <= 'z') || (str[0] >= 'A' && str[0] <= 'Z');
}

inline bool isInt(const std::string& str)
{
    return str.length() == 1 && isdigit(str[0]);
}

inline bool isSkippable(const std::string str)
{
    return str == " " || str == "\t" || str == "\r" || str == "";
}

inline std::unordered_map<std::string, TokenType> getKeyWords()
{
    static std::unordered_map<std::string, TokenType> keywords =
    {
        { "var", Probescript::Lexer::TokenType::Var },
        { "null", Probescript::Lexer::TokenType::Null },
        { "const", Probescript::Lexer::TokenType::Const },
        { "fn", Probescript::Lexer::TokenType::Function },
        { "if", Probescript::Lexer::TokenType::If },
        { "probe", Probescript::Lexer::TokenType::Probe },
        { "import", Probescript::Lexer::TokenType::Import },
        { "export", Probescript::Lexer::TokenType::Export },
        { "module", Probescript::Lexer::TokenType::Module },
        { "while", Probescript::Lexer::TokenType::While },
        { "else", Probescript::Lexer::TokenType::Else },
        { "class", Probescript::Lexer::TokenType::Class },
        { "new", Probescript::Lexer::TokenType::New },
        { "return", Probescript::Lexer::TokenType::Return },
        { "extends", Probescript::Lexer::TokenType::Extends },
        { "as", Probescript::Lexer::TokenType::As },
        { "for", Probescript::Lexer::TokenType::For },
        { "break", Probescript::Lexer::TokenType::Break },
        { "continue", Probescript::Lexer::TokenType::Continue },
        { "throw", Probescript::Lexer::TokenType::Throw },
        { "try", Probescript::Lexer::TokenType::Try },
        { "catch", Probescript::Lexer::TokenType::Catch },
        { "undefined", Probescript::Lexer::TokenType::Undefined },
        { "true", Probescript::Lexer::TokenType::Bool },
        { "false", Probescript::Lexer::TokenType::Bool },
        { "async", Probescript::Lexer::TokenType::Async },
        { "await", Probescript::Lexer::TokenType::Await },
    };

    return keywords;
}

std::vector<Token> tokenize(const std::string& sourceCode);

} // namespace Probescript::Lexer