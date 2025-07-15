#pragma once

#include "core/runtime/values.hpp"
#include "core/env.hpp"
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
    JSONParser(std::string& file, EnvPtr env = std::make_shared<Env>()) : file(file), env(env) {}
    Values::Val parse();

private:
    EnvPtr env;
    std::string& file;
    std::vector<Token> tokens;
    bool tokenize();

    inline bool skippable(std::string& s) {
        return (s == " " || s == "" || s == "\n" || s == "\t" || s == "\r");
    }

    inline bool isInt(const std::string& str) {
        return str.length() == 1 && isdigit(str[0]);
    }

    inline Values::Val parseTokens() {
        return parseValue();
    }
    
    Values::Val parseValue();
    
    Values::Val parseObject();

    Values::Val parseArray();

    inline Token eat() {
        return shift(tokens);
    }
};

Values::Val getValJsonModule();
Typechecker::TypePtr getTypeJsonModule();

} // namespace Probescript::Stdlib::JSON