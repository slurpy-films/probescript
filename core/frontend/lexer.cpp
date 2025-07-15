#include "frontend/lexer.hpp"

using namespace Probescript;
using namespace Probescript::Lexer;

std::vector<Token> Lexer::tokenize(const std::string& sourceCode)
{
    std::vector<Token> tokens;
    std::vector<std::string> src = splitToChars(sourceCode);
    int line = 1;
    int col = 1;

    std::unordered_map<std::string, TokenType> keywords = getKeyWords();

    std::unordered_map<std::string, TokenType> singleCharTokens =
    {
        { "(", OpenParen }, { ")", ClosedParen },
        { "{", OpenBrace }, { "}", ClosedBrace },
        { "[", OpenBracket }, { "]", CloseBracket },
        { ",", Comma }, { ":", Colon }, { ".", Dot },
        { "+", BinaryOperator }, { "-", BinaryOperator },
        { "*", BinaryOperator }, { "/", BinaryOperator },
        { "%", BinaryOperator }, { "=", Equals },
        { "<", LessThan}, { ">", GreaterThan },
        { "!", Bang }, { "?", Ternary },
    };

    std::vector<std::pair<std::string, TokenType>> multiCharTokens =
    {
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

    while (!src.empty())
    {
        if (src[0] == "\n")
        {
            line++;
            col = 1;
            shift(src);
            continue;
        }

        if (isSkippable(src[0]))
        {
            if (src[0] != "") col++;
            shift(src);
            continue;
        }

        if (src[0] == ";")
        {
            col++;
            tokens.push_back(token(shift(src), Semicolon, { line, col }));
            continue;
        }

        if (!src.empty() && src[0] == "/" && src.size() > 1 && src[1] == "/")
        {
            shift(src);
            shift(src);
            col += 2;
            while (!src.empty() && src[0] != "\n")
            {
                col++;
                shift(src);
            }
            continue;
        }        

        if (isInt(src[0]) || ((src[0] == "-" || src[0] == ".") && isInt(src[1])))
        {
            std::string num = "";
            int tokenLine = line;
            int tokenCol = col;

            while (!src.empty() && (isInt(src[0]) || ((src[0] == "-" || src[0] == ".") && isInt(src[1]))))
            {
                col++;
                num += shift(src);
            }

            tokens.push_back(token(num, Number, { tokenLine, tokenCol }));
            continue;
        }

        bool matchedMulti = false;
        for (const auto& [symbol, type] : multiCharTokens)
        {
            if (src.size() >= symbol.length())
            {
                std::string joined = "";
                for (size_t i = 0; i < symbol.length(); ++i)
                    joined += src[i];

                if (joined == symbol)
                {
                    int tokenLine = line;
                    int tokenCol = col;

                    for (size_t i = 0; i < symbol.length(); ++i)
                    {
                        col++;
                        shift(src);
                    }
                    tokens.push_back(token(symbol, type, { tokenLine, tokenCol }));
                    matchedMulti = true;
                    break;
                }
            }
        }
        if (matchedMulti) continue;

        if (singleCharTokens.find(src[0]) != singleCharTokens.end())
        {
            tokens.push_back(token(src[0], singleCharTokens[src[0]], { line, col }));
            shift(src);
            col++;
            continue;
        }

        if (src[0] == "\"" || src[0] == "'")
        {
            col++;
            int tokenLine = line;
            int tokenCol = col;
            std::string quoteType = shift(src);
            std::string value = "";
        
            while (!src.empty() && src[0] != quoteType)
            {
                std::string val = shift(src);
                col++;
                if (val == "\\") {
                    if (src.empty()) break;
                    col++;
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
        
            if (!src.empty()) { shift(src); col++; }
            tokens.push_back(token(value, String, { tokenLine, tokenCol }));
            continue;
        }

        if (isAlpha(src[0]) || src[0] == "_")
        {
            std::string ident = "";
            int l = 0;
            while (!src.empty() && (isAlpha(src[0]) || src[0] == "_"))
            {
                ident += shift(src);
                l++;
            }
            if (keywords.find(ident) != keywords.end())
            {
                tokens.push_back(token(ident, keywords[ident], { line, col }));
            }
            else
            {
                tokens.push_back(token(ident, Identifier, { line, col }));
            }
            col += l;
            continue;
        }

        throw std::runtime_error("Unrecognized character in source: '" + src[0] + "'\n");
    }

    tokens.push_back(token("EndOfFile", END, { line, col }));

    return tokens;
}