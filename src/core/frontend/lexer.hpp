#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "utils.hpp"
#include <iostream>

namespace Lexer {

enum TokenType {
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
    Openbrace,
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
};

struct Token {
    std::string value;
    TokenType type;

    int line;
    int col;
    std::string file;
};

inline Token token(const std::string& value, const TokenType type, const std::pair<int, int> pos) {
    int line = pos.first;
    int col = pos.second;
    
    return { value, type, line, col };
}

inline bool isAlpha(const std::string& str) {
    return (str[0] >= 'a' && str[0] <= 'z') || (str[0] >= 'A' && str[0] <= 'Z');
}

inline bool isInt(const std::string& str) {
    return str.length() == 1 && isdigit(str[0]);
}

inline bool isSkippable(const std::string str) {
    return str == " " || str == "\t" || str == "\r" || str == "";
}

inline std::unordered_map<std::string, TokenType> getKeyWords() {
    static std::unordered_map<std::string, TokenType> keywords = {
        { "var", Var },
        { "null", Null },
        { "const", Const },
        { "fn", Function },
        { "if", If },
        { "probe", Probe },
        { "import", Import },
        { "export", Export },
        { "module", Module },
        { "while", While },
        { "else", Else },
        { "class", Class },
        { "new", New },
        { "return", Return },
        { "extends", Extends },
        { "as", As },
        { "for", For },
        { "break", Break },
        { "continue", Continue },
        { "throw", Throw },
        { "try", Try },
        { "catch", Catch },
        { "undefined", Undefined },
        { "true", Bool },
        { "false", Bool }
    };

    return keywords;
}

std::vector<Token> tokenize(const std::string& sourceCode);

} // namespace Lexer