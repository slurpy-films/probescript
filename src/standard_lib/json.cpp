#include "json.hpp"

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