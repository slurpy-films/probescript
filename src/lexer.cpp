#include "lexer.hpp"

using namespace Lexer;

std::vector<Token> Lexer::tokenize(const std::string& sourceCode) {
    std::vector<Token> tokens;
    std::vector<std::string> src = splitToChars(sourceCode);

    std::unordered_map<std::string, TokenType> keywords = getKeyWords();

    std::unordered_map<std::string, TokenType> singleCharTokens = {
        { "(", OpenParen }, { ")", ClosedParen },
        { "{", Openbrace }, { "}", ClosedBrace },
        { "[", OpenBracket }, { "]", CloseBracket },
        { ",", Comma }, { ":", Colon }, { ".", Dot },
        { "+", BinaryOperator }, { "-", BinaryOperator },
        { "*", BinaryOperator }, { "/", BinaryOperator },
        { "%", BinaryOperator }, { "=", Equals },
        { "<", BinaryOperator }, { ">", BinaryOperator },
        { "!", Bang },
    };

    std::vector<std::pair<std::string, TokenType>> multiCharTokens = {
        { "&&", AndOperator },
        { "||", OrOperator },
        { "==", DoubleEquals },
        { "!=", NotEquals },
        { "<=", BinaryOperator },
        { ">=", BinaryOperator },
        { "+=", AssignmentOperator },
        { "-=", AssignmentOperator },
        { "*=", AssignmentOperator },
        { "/=", AssignmentOperator },
        { "++", Increment },
        { "--", Decrement },
        { "=>", Arrow },
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

        if (!src.empty() && src[0] == "/" && src.size() > 1 && src[1] == "/") {
            shift(src);
            shift(src);
            while (!src.empty() && src[0] != "\n") shift(src);
            continue;
        }        

        if (isInt(src[0]) || ((src[0] == "-" || src[0] == ".") && isInt(src[1]))) {
            std::string num = "";
            while (!src.empty() && (isInt(src[0]) || ((src[0] == "-" || src[0] == ".") && isInt(src[1])))) {
                num += shift(src);
            }
            tokens.push_back(token(num, Number));
            continue;
        }

        bool matchedMulti = false;
        for (const auto& [symbol, type] : multiCharTokens) {
            if (src.size() >= symbol.length()) {
                std::string joined = "";
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
            tokens.push_back(token(src[0], singleCharTokens[src[0]]));
            shift(src);
            continue;
        }

        if (src[0] == "\"" || src[0] == "'") {
            std::string quoteType = shift(src);
            std::string value = "";
        
            while (!src.empty() && src[0] != quoteType) {
                std::string val = shift(src);
                if (val == "\\") {
                    if (src.empty()) break;
                    char c = shift(src)[0];
                    if (c == 'n') value += '\n';
                    else if (c == 't') value += '\t';
                    else if (c == '\\') value += '\\';
                    else if (c == '"') value += '"';
                    else if (c == '\'') value += '\'';
                    else if (c == 'r') value += '\r';
                    else if (c == 'b') value += '\b';
                    else if (c == 'f') value += '\f';
                    else value += c;
                } else {
                    value += val;
                }
            }
        
            if (!src.empty()) shift(src);
            tokens.push_back(token(value, String));
            continue;
        }
        

        if (isAlpha(src[0])) {
            std::string ident = "";
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

        std::cout << "Unrecognized character in source: '" << src[0] << "'" << std::endl;
        exit(1);
    }

    tokens.push_back(token("EndOfFile", END));

    return tokens;
}