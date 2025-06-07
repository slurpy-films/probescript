#pragma once
#include "runtime/values.hpp"
#include "env.hpp"
#include "utils.hpp"
#include "errors.hpp"
#include "types.hpp"

namespace JSON {

enum TokenType {
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

struct Token {
    std::string val;
    TokenType type;

    Token(std::string v, TokenType t)
        : val(v), type(t) {}
};

class JSONParser {
public:
    JSONParser(std::string& file, EnvPtr env = std::make_shared<Env>()) : file(file), env(env) {}
    Val parse();

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

    inline Val parseTokens() {
        return parseValue();
    }
    
    Val parseValue();
    
    Val parseObject();

    Val parseArray();

    inline Token eat() {
        return shift(tokens);
    }
};

} // namespace JSON


Val getValJsonModule();
TypePtr getTypeJsonModule();