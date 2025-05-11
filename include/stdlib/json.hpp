#pragma once
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "utils/split.hpp"
#include "utils/shift.hpp"

namespace JSONParser {

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
    JSONParser(std::string& file) : file(file) {}
    Val parse() {
        tokenize();
        return parseTokens();
    }

private:
    std::string& file;
    std::vector<Token> tokens;
    void tokenize() {
        std::vector<std::string> src = splitToChars(file);
        std::vector<Token> tokens;
        while (!src.empty()) {
            if (skippable(src[0])) {
                shift(src);
                continue;
            }

            if (src[0] == "\"") {
                shift(src);
                std::string r;
                while (!src.empty() && src[0] != "\"") {
                    r += shift(src);
                }

                if (src.empty()) {
                    std::cerr << "Expected end \" after string";
                    exit(1);
                }

                shift(src);

                tokens.push_back(Token(r, String));
                continue;
            }

            if (src[0] == "t" && src[1] == "r" && src[2] == "u" && src[3] == "e") {
                shift(src);
                shift(src);
                shift(src);
                shift(src);

                tokens.push_back(Token("true", Boolean));
                continue;
            }

            if (src[0] == "f" && src[1] == "a" && src[2] == "l" && src[3] == "s" && src[4] == "e") {
                shift(src);
                shift(src);
                shift(src);
                shift(src);
                shift(src);

                tokens.push_back(Token("false", Boolean));
                continue;
            }

            if (isInt(src[0]) || ((src[0] == "-" || src[0] == ".") && isInt(src[1]))) {
                std::string num = "";
                while (!src.empty() && (isInt(src[0]) || ((src[0] == "-" || src[0] == ".") && isInt(src[1])))) {
                    num += shift(src);
                }
                tokens.push_back(Token(num, Number));
                continue;
            }

            if (src[0] == "{") {
                tokens.push_back(Token(shift(src), OpenBrace));
                continue;
            } else if (src[0] == "}") {
                tokens.push_back(Token(shift(src), ClosedBrace));
                continue;
            } else if (src[0] == ":") {
                tokens.push_back(Token(shift(src), Colon));
                continue;
            } else if (src[0] == ",") {
                tokens.push_back(Token(shift(src), Comma));
                continue;
            }  else if (src[0] == "[") {
                tokens.push_back(Token(shift(src), OpenBracket));
            } else if (src[0] == "]") {
                tokens.push_back(Token(shift(src), CloseBracket));
            } else {
                std::cerr << "Unknown sign in JSON: '" << src[0] << "'";
                exit(1);
            }
        }

        this->tokens = tokens;
    }

    bool skippable(std::string& s) {
        return (s == " " || s == "" || s == "\n" || s == "\t" || s == "\r");
    }

    bool isInt(const std::string& str) {
        return str.length() == 1 && isdigit(str[0]);
    }

    Val parseTokens() {
        return parseValue();
    }
    
    Val parseValue() {
        switch (tokens[0].type) {
            case TokenType::String:
                return std::make_shared<StringVal>(eat().val);
            case TokenType::Number:
                return std::make_shared<NumberVal>(eat().val);
            case TokenType::OpenBrace:
                return parseObject();
            case TokenType::Boolean:
                return std::make_shared<BooleanVal>(eat().val == "true");
            case TokenType::OpenBracket:
                return parseArray();
            default:
                std::cerr << "Unknown value type in JSON: " << tokens[0].type;
                exit(1);
        }
    }
    
    Val parseObject() {
        eat();
        std::shared_ptr<ObjectVal> o = std::make_shared<ObjectVal>();

        if (tokens[0].type != TokenType::String) {
            std::cerr << "Expected object key to be of type string";
            exit(1);
        }
        std::string key = eat().val;

        eat();
        Val val = parseValue();
        o->properties[key] = val;
        while (tokens[0].type != TokenType::ClosedBrace) {
            if (!tokens[0].type == TokenType::Comma) {
                std::cerr << "Expected comma after object property";
                exit(1);
            }
            eat();
            if (tokens[0].type != TokenType::String) {
                std::cerr << "Expected object key to be of type string";
                exit(1);
            }
            std::string key = eat().val;
            eat();
            Val val = parseValue();
            o->properties[key] = val;
        }

        eat();
        return o;
    }

    Val parseArray() {
        eat();
        std::vector<Val> items;
        items.push_back(parseValue());
        while (!tokens.empty() && tokens[0].type == TokenType::Comma) {
            eat();
            items.push_back(parseValue());
        }

        if (tokens[0].type != TokenType::CloseBracket) {
            std::cerr << "Expected closing bracket after array";
            exit(1);
        }
        eat();

        return std::make_shared<ArrayVal>(items);
    }

    Token eat() {
        return shift(tokens);
    }
};

} // namespace JSONParser


std::unordered_map<std::string, Val> getJsonModule() {
    return {
        {
            "parse",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
                if (args.empty() || args[0]->type != ValueType::String) return std::make_shared<UndefinedVal>();

                JSONParser::JSONParser parser(std::static_pointer_cast<StringVal>(args[0])->string);
                return parser.parse();
            })
        },
        {
            "stringify",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, Env* env) -> Val {
                if (args.empty() || args[0]->type != ValueType::Object) return std::make_shared<UndefinedVal>();
                return std::make_shared<StringVal>(args[0]->toJSON());
            })
        }
    };
}