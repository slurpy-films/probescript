#include "json.hpp"

using namespace JSON;

Val JSONParser::parse() {
    if (tokenize()) return std::make_shared<UndefinedVal>();
    return parseTokens();
}

bool JSONParser::tokenize() {
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
                throw ThrowException(CustomError("Expected end \" after string", "JsonError"));
                return true;
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
            throw ThrowException(CustomError("Unknown sign in JSON: '" + src[0] + "'", "JsonError"));
            return true;
        }
    }

    this->tokens = tokens;
    return false;
}

Val JSONParser::parseValue() {
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
            throw ThrowException(CustomError("Unknown value type in JSON", "JsonError"));
    }
}

Val JSONParser::parseObject() {
    eat();
    std::shared_ptr<ObjectVal> o = std::make_shared<ObjectVal>();
    if (tokens[0].type == TokenType::ClosedBrace) {
        eat();
        return std::make_shared<ObjectVal>();
    }

    if (tokens[0].type != TokenType::String) {
        throw ThrowException(CustomError("Expected object key to be of type string", "JsonError"));
    }
    std::string key = eat().val;

    eat();
    Val val = parseValue();
    o->properties[key] = val;
    while (tokens[0].type != TokenType::ClosedBrace) {
        if (tokens[0].type != TokenType::Comma) {
            throw ThrowException(CustomError("Expected comma after object property", "JsonError"));
        }
        eat();
        if (tokens[0].type != TokenType::String) {
            throw ThrowException(CustomError("Expected object key to be of type string", "JsonError"));
        }
        std::string key = eat().val;
        eat();
        Val val = parseValue();
        o->properties[key] = val;
    }

    eat();
    return o;
}

Val JSONParser::parseArray() {
    eat();
    std::vector<Val> items;
    items.push_back(parseValue());
    while (!tokens.empty() && tokens[0].type == TokenType::Comma) {
        eat();
        items.push_back(parseValue());
    }

    if (tokens[0].type != TokenType::CloseBracket) {
        throw ThrowException(CustomError("Expected closing bracket after array", "JsonError"));
    }
    eat();

    return std::make_shared<ArrayVal>(items);
}

Val getValJsonModule()
{
    return
    std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>({
        {
            "parse",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.empty() || args[0]->type != ValueType::String) throw ThrowException(ArgumentError("Expected argument 1 to be of type string"));

                JSON::JSONParser parser(std::static_pointer_cast<StringVal>(args[0])->string, env);
                return parser.parse();
            })
        },
        {
            "to_string",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.empty() || args[0]->type != ValueType::Object) throw ThrowException(ArgumentError("Expected argument 1 to be of type object"));
                return std::make_shared<StringVal>(args[0]->toJSON());
            })
        }
    }));
}

TypePtr getTypeJsonModule()
{
    return
    std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>(
    {
        {
            "parse",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ std::make_shared<VarDeclarationType>(std::make_shared<UndefinedLiteralType>(), "raw", std::make_shared<IdentifierType>("str")) })))
        },
        {
            "to_string",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ std::make_shared<VarDeclarationType>(std::make_shared<UndefinedLiteralType>(), "object") })))
        }
    })));
}