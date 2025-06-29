#include "globals.hpp"

std::unordered_map<std::string, std::pair<Val, TypePtr>> g_globals =
{
    {
        "console",
        {
            std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>({
                {
                    "println",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                        for (Val arg : args) {
                            std::cout << (arg->type == ValueType::Object ? arg->toConsole() : arg->toString()) << " ";
                        }

                        std::cout << std::endl;
                        return std::make_shared<UndefinedVal>();
                    })
                },
                {
                    "print",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                        for (Val arg : args) {
                            std::cout << (arg->type == ValueType::Object ? arg->toConsole() : arg->toString()) << " ";
                        }

                        return std::make_shared<UndefinedVal>();
                    })
                },
                {
                    "prompt",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                        for (Val arg : args) {
                            std::cout << arg->toString();
                        }

                        std::string input;
                        std::getline(std::cin, input);

                        return std::make_shared<StringVal>(input);
                    })
                }
            })),
            std::make_shared<Type>(TypeKind::Module, "module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({ { "println", std::make_shared<Type>(TypeKind::Function, "native function") }, { "print", std::make_shared<Type>(TypeKind::Function, "native function") }, { "prompt", std::make_shared<Type>(TypeKind::Function, "native function")} })))
        }
    },
    {
        "num",
        {
            std::make_shared<NativeClassVal>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (!args.empty() && !isNum(args[0]->toString()))
                {
                    return env->throwErr(ArgumentError("Invalid argument: " + args[0]->toString() + " is not a number"));
                }

                return std::make_shared<NumberVal>(!args.empty() ? args[0]->toNum() : 0);
            }),
            std::make_shared<Type>(TypeKind::Class, "native class")
        }
    },
    {
        "str",
        {
            std::make_shared<NativeClassVal>([](std::vector<Val> args, EnvPtr env) -> Val {
                return std::make_shared<StringVal>(!args.empty() ? args[0]->toString() : "");
            }),
            std::make_shared<Type>(TypeKind::Class, "native class")
        }
    },
    {
        "bool",
        {
            std::make_shared<NativeClassVal>([](std::vector<Val> args, EnvPtr env) -> Val {
                return std::make_shared<BooleanVal>(!args.empty() ? args[0]->toBool() : false);
            }),
            std::make_shared<Type>(TypeKind::Class, "native class")
        }
    },
    {
        "map",
        {
            std::make_shared<NativeClassVal>([](std::vector<Val> args, EnvPtr env) -> Val {
                return std::make_shared<ObjectVal>(args.empty() ? std::unordered_map<std::string, Val>() : args[0]->properties);
            }),
            std::make_shared<Type>(TypeKind::Class, "native class")
        }
    },
    {
        "function",
        {
            std::make_shared<NativeClassVal>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.empty() || args[0]->type == ValueType::Function) return env->throwErr(ArgumentError("Usage: new function(fn: function)"));

                return std::static_pointer_cast<FunctionValue>(args[0]);
            }),
            std::make_shared<Type>(TypeKind::Class, "native class")
        }
    },
    {
        "array",
        {
            std::make_shared<NativeClassVal>([](std::vector<Val> args, EnvPtr env) -> Val {
                std::shared_ptr<ArrayVal> array = std::make_shared<ArrayVal>();

                for (Val val : args)
                    array->items.push_back(val);

                return array;
            }),
            std::make_shared<Type>(TypeKind::Class, "native class")
        }
    },
    {
        "process",
        {
            std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>(
            {
                {
                    "kill",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
                    {
                        exit(args.empty() ? 0 : args[0]->toNum());
                        return std::make_shared<UndefinedVal>();
                    })
                }
            })),
            std::make_shared<Type>(TypeKind::Any, "native module")
        }
    },
    {
        "keys",
        {
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.empty() || args[0]->type != ValueType::Object) return env->throwErr(ArgumentError("Usage: keys(obj: Object)"));

                std::shared_ptr<ArrayVal> array = std::make_shared<ArrayVal>();

                for (const auto& [key, _] : args[0]->properties)
                {
                    array->items.push_back(std::make_shared<StringVal>(key));
                }

                return array;
            }),
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "obj", new IdentifierType("map")) })))
        }
    },
    {
        "values",
        {
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.empty() || args[0]->type != ValueType::Object) return env->throwErr(ArgumentError("Usage: values(obj: Object)"));

                std::shared_ptr<ArrayVal> array = std::make_shared<ArrayVal>();

                for (const auto& [_, val] : args[0]->properties)
                {
                    array->items.push_back(val);
                }

                return array;
            }),
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "obj", new IdentifierType("map")) })))
        }
    },
    {
        "copy",
        {
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.empty()) return env->throwErr(ArgumentError("Usage: copy(val: any)"));

                return std::make_shared<RuntimeVal>(*args[0]);
            }),
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "val") })))
        }
    }
};