#pragma once

#include "core/vm/values.hpp"

#include "core/utils.hpp"
#include "core/errors.hpp"
#include "core/types.hpp"

namespace Probescript::Stdlib::JSON
{

enum TokenType
{
    String,
    OpenBrace,
    ClosedBrace,
    Number,
    Colon,
    Comma,
    Boolean,
    OpenBracket,
    CloseBracket,
};

struct Token
{
    std::string val;
    TokenType type;

    Token(std::string v, TokenType t)
        : val(v), type(t) {}
};

class JSONParser
{
public:
    JSONParser(std::string& file) : file(file) {}
    VM::ValuePtr parse();

private:
    std::string& file;
    std::vector<Token> tokens;
    bool tokenize();

    inline bool skippable(std::string& s) {
        return (s == " " || s == "" || s == "\n" || s == "\t" || s == "\r");
    }

    inline bool isInt(const std::string& str) {
        return str.length() == 1 && isdigit(str[0]);
    }

    inline VM::ValuePtr parseTokens() {
        return parseValue();
    }
    
    VM::ValuePtr parseValue();
    
    VM::ValuePtr parseObject();

    VM::ValuePtr parseArray();

    inline Token eat() {
        return shift(tokens);
    }
};

VM::ValuePtr getValJsonModule();
Typechecker::TypePtr getTypeJsonModule();

} // namespace Probescript::Stdlib::JSON