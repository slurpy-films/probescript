#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "utils/split.hpp"
#include "utils/shift.hpp"
#include <iostream>

using namespace std;

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
    AndOperator,
    OrOperator,
    DoubleEquals,
    NotEquals,
    Import,
    Export,
    Module,
};

struct Token {
    string value;
    TokenType type;
};

Token token(const string& value, const TokenType type) {
    return { value, type };
}

bool isAlpha(const string& str) {
    return (str[0] >= 'a' && str[0] <= 'z') || (str[0] >= 'A' && str[0] <= 'Z');
}

bool isInt(const string& str) {
    return str.length() == 1 && isdigit(str[0]);
}

bool isSkippable(const string str) {
    return str == " " || str == "\t" || str == "\n" || str == "\r";
}

unordered_map<string, TokenType> getKeyWords() {
    static unordered_map<string, TokenType> keywords = {
        { "var", Var },
        { "null", Null },
        { "const", Const },
        { "fn", Function },
        { "if", If },
        { "probe", Probe },
        { "import", Import },
        { "export", Export },
        { "module", Module }, 
    };

    return keywords;
}

vector<Token> tokenize(const string& sourceCode) {
    vector<Token> tokens;
    vector<string> src = splitToChars(sourceCode);

    unordered_map<string, TokenType> keywords = getKeyWords();

    unordered_map<string, TokenType> singleCharTokens = {
        { "(", OpenParen }, { ")", ClosedParen },
        { "{", Openbrace }, { "}", ClosedBrace },
        { "[", OpenBracket }, { "]", CloseBracket },
        { ",", Comma }, { ":", Colon }, { ".", Dot },
        { "+", BinaryOperator }, { "-", BinaryOperator },
        { "*", BinaryOperator }, { "/", BinaryOperator },
        { "%", BinaryOperator }, { "=", Equals },
        { "<", BinaryOperator }, { ">", BinaryOperator }
    };

    vector<pair<string, TokenType>> multiCharTokens = {
        { "&&", AndOperator },
        { "||", OrOperator },
        { "==", DoubleEquals },
        { "!=", NotEquals },
        { "<=", BinaryOperator },
        { ">=", BinaryOperator },
    };

    while (!src.empty()) {
        if (isSkippable(src[0])) {
            shift(src);
            continue;
        }

        if (src[0] == ";") {
            shift(src);
            continue;
        }

        bool matchedMulti = false;
        for (const auto& [symbol, type] : multiCharTokens) {
            if (src.size() >= symbol.length()) {
                string joined = "";
                for (size_t i = 0; i < symbol.length(); ++i)
                    joined += src[i];

                if (joined == symbol) {
                    for (size_t i = 0; i < symbol.length(); ++i)
                        shift(src);
                    tokens.push_back(token(symbol, type));
                    matchedMulti = true;
                    break;
                }
            }
        }
        if (matchedMulti) continue;

        if (singleCharTokens.find(src[0]) != singleCharTokens.end()) {
            tokens.push_back(token(shift(src), singleCharTokens[src[0]]));
            continue;
        }

        if (src[0] == "\"") {
            shift(src);
            string value = "";
            while (!src.empty() && src[0] != "\"") {
                if (src[0] == "\n") {
                    cerr << "Expected closing quote";
                    exit(1);
                }
                value += shift(src);
            }
            if (!src.empty()) shift(src);
            tokens.push_back(token(value, String));
            continue;
        }

        if (isInt(src[0])) {
            string num = "";
            while (!src.empty() && isInt(src[0])) {
                num += shift(src);
            }
            tokens.push_back(token(num, Number));
            continue;
        }

        if (isAlpha(src[0])) {
            string ident = "";
            while (!src.empty() && isAlpha(src[0])) {
                ident += shift(src);
            }
            if (keywords.find(ident) != keywords.end()) {
                tokens.push_back(token(ident, keywords[ident]));
            } else {
                tokens.push_back(token(ident, Identifier));
            }
            continue;
        }

        cout << "Unrecognized character in source: " << src[0] << endl;
        exit(1);
    }

    tokens.push_back(token("EndOfFile", END));
    return tokens;
}

}