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
            std::make_shared<Type>(TypeKind::Module, "module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({ { "println", std::make_shared<Type>(TypeKind::Function, "native function") }, { "prompt", std::make_shared<Type>(TypeKind::Function, "native function")} })))
        }
    },
    {
        "num",
        {
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.empty()) return env->throwErr(ArgumentError("Usage: num(val: any)"));
                if (!isNum(args[0]->toString()))
                {
                    return env->throwErr(ArgumentError("Invalid argument: " + args[0]->toString() + " is not a number"));
                }

                return std::make_shared<NumberVal>(args[0]->toNum());
            }),
            std::make_shared<Type>(TypeKind::Function, "native function")
        }
    },
    {
        "str",
        {
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.empty()) return env->throwErr(ArgumentError("Usage: str(val: any)"));

                return std::make_shared<StringVal>(args[0]->toString());
            }),
            std::make_shared<Type>(TypeKind::Function, "native function")
        }
    },
    {
        "process",
        {
            std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>(
            {
                {
                    "kill",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                        exit(args.empty() ? 0 : args[0]->toNum());
                        return std::make_shared<UndefinedVal>();
                    })
                }
            })),
            std::make_shared<Type>(TypeKind::Any, "native module")
        }
    }
};