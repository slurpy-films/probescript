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
    NewLine,
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
    return str == " " || str == "\t" || str == "\r";
}

unordered_map<string, TokenType> getKeyWords() {
    static unordered_map<string, TokenType> keywords = {
        { "var", Var },
        { "null", Null },
        { "const", Const },
        { "function", Function },
    };

    return keywords;
}

vector<Token> tokenize(const string& sourceCode) {
    vector<Token> tokens;
    vector<string> src = splitToChars(sourceCode);

    unordered_map<string, TokenType> keywords = getKeyWords();
    while (src.size() > 0) {
        if (isSkippable(src[0])) {
            shift(src);
        } else if (src[0] == "\n") {
            tokens.push_back(token(shift(src), NewLine));
        } else if (src[0] == "(") {
            tokens.push_back(token(shift(src), OpenParen));
        } else if (src[0] == ")") {
            tokens.push_back(token(shift(src), ClosedParen));
        } else if (src[0] == "{") {
            tokens.push_back(token(shift(src), Openbrace));
        } else if (src[0] == "}") {
            tokens.push_back(token(shift(src), ClosedBrace));
        } else if (src[0] == "[") {
            tokens.push_back(token(shift(src), OpenBracket));
        } else if (src[0] == "]") {
            tokens.push_back(token(shift(src), CloseBracket));
        } else if (src[0] == ",") {
            tokens.push_back(token(shift(src), Comma));
        } else if (src[0] == ":") {
            tokens.push_back(token(shift(src), Colon));
        } else if (src[0] == "+" || src[0] == "-" || src[0] == "*" || src[0] == "/" || src[0] == "%") {
            tokens.push_back(token(shift(src), BinaryOperator));
        } else if (src[0] == "=") {
            tokens.push_back(token(shift(src), Equals));
        } else if (src[0] == ";") {
            tokens.push_back(token(shift(src), Semicolon));
        } else if (src[0] == ".") {
            tokens.push_back(token(shift(src), Dot));
        } else if (src[0] == "\"") {
            shift(src);
            string value = "";
            while (src[0] != "\"") {
                if (src[0] == "\n" || src.size() <= 0) {
                    cerr << "Expected closing quote";
                    exit(1);
                } 
                value += shift(src);
            }

            shift(src);
            tokens.push_back(token(value, String));
        } else if (isInt(src[0])) {
            string num = "";
            while (src.size() > 0 && isInt(src[0])) {
                num += shift(src);
            }
            tokens.push_back(token(num, Number));
        } else if (isAlpha(src[0])) {
            string ident = "";
            while (src.size() > 0 && isAlpha(src[0])) {
                ident += shift(src);
            }
            if (keywords.find(ident) != keywords.end()) {
                tokens.push_back(token(ident, keywords[ident]));
            } else {
                tokens.push_back(token(ident, Identifier));
            }
        } else {
            cout << "Unrecognized character found in source: ";
            cout << src[0] << endl;
            exit(1);
        }
    }
    

    tokens.push_back(token("EndOfFile", END));

    return tokens;
};

}