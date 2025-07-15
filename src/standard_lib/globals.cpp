#include "globals.hpp"

using namespace Probescript;

std::unordered_map<std::string, std::pair<Values::Val, Typechecker::TypePtr>> g_globals =
{
    {
        "console",
        {
            std::make_shared<Values::ObjectVal>(std::unordered_map<std::string, Values::Val>({
                {
                    "println",
                    std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                        for (Values::Val arg : args) {
                            std::cout << (arg->type == Values::ValueType::Object ? arg->toConsole() : arg->toString()) << " ";
                        }

                        std::cout << std::endl;
                        return std::make_shared<Values::UndefinedVal>();
                    })
                },
                {
                    "print",
                    std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                        for (Values::Val arg : args) {
                            std::cout << (arg->type == Values::ValueType::Object ? arg->toConsole() : arg->toString()) << " ";
                        }

                        return std::make_shared<Values::UndefinedVal>();
                    })
                },
                {
                    "prompt",
                    std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                        for (Values::Val arg : args) {
                            std::cout << arg->toString();
                        }

                        std::string input;
                        std::getline(std::cin, input);

                        return std::make_shared<Values::StringVal>(input);
                    })
                }
            })),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Module, "module", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>({ { "println", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function") }, { "print", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function") }, { "prompt", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function")} })))
        }
    },
    {
        "num",
        {
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (!args.empty() && !isNum(args[0]->toString()))
                {
                    throw ThrowException(ArgumentError("Invalid argument: '" + args[0]->toString() + "' is not a number"));
                }

                return std::make_shared<Values::NumberVal>(!args.empty() ? args[0]->toNum() : 0);
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Number, "number")))
        }
    },
    {
        "str",
        {
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                return std::make_shared<Values::StringVal>(!args.empty() ? args[0]->toString() : "");
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::String, "string")))
        }
    },
    {
        "bool",
        {
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                return std::make_shared<Values::BooleanVal>(!args.empty() ? args[0]->toBool() : false);
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Bool, "boolean")))
        }
    },
    {
        "map",
        {
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                return std::make_shared<Values::ObjectVal>(args.empty() ? std::unordered_map<std::string, Values::Val>() : args[0]->properties);
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Object, "map")))
        }
    },
    {
        "function",
        {
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                if (args.empty() || args[0]->type == Values::ValueType::Function) throw ThrowException(ArgumentError("Usage: new function(fn: function)"));

                return std::static_pointer_cast<Values::FunctionValue>(args[0]);
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function")))
        }
    },
    {
        "array",
        {
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val {
                std::shared_ptr<Values::ArrayVal> array = std::make_shared<Values::ArrayVal>();

                for (Values::Val val : args)
                    array->items.push_back(val);

                return array;
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Array, "array")))
        }
    },
    {
        "exit",
        {
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                exit(args.empty() ? 0 : args[0]->toNum());
                return std::make_shared<Values::UndefinedVal>();
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Any, "native module", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("code", Typechecker::g_numty, true) })))
        }
    },
    {
        "keys",
        {
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.empty() || args[0]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: keys(obj: Object)"));

                std::shared_ptr<Values::ArrayVal> array = std::make_shared<Values::ArrayVal>();

                for (const auto& [key, _] : args[0]->properties)
                {
                    array->items.push_back(std::make_shared<Values::StringVal>(key));
                }

                return array;
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("obj", Typechecker::g_anyty, false) })))
        }
    },
    {
        "values",
        {
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.empty() || args[0]->type != Values::ValueType::Object) throw ThrowException(ArgumentError("Usage: values(obj: Object)"));

                std::shared_ptr<Values::ArrayVal> array = std::make_shared<Values::ArrayVal>();

                for (const auto& [_, val] : args[0]->properties)
                {
                    array->items.push_back(val);
                }

                return array;
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("obj", Typechecker::g_anyty, false) })))
        }
    },
    {
        "copy",
        {
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.empty()) throw ThrowException(ArgumentError("Usage: copy(val: any)"));

                return std::make_shared<Values::RuntimeVal>(*args[0]);
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("val", Typechecker::g_anyty, false) })))
        }
    },
    {
        "evaluate",
        {
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.empty()) throw ThrowException(ArgumentError("Usage: evaluate(val: string)"));

                try
                {
                    std::string code = args[0]->toString();
                    return Interpreter::eval(Parser().parse(code), std::make_shared<Env>(), std::make_shared<Context>(RuntimeType::REPL));
                }
                catch (const std::runtime_error& err)
                {
                    throw ThrowException(err.what());
                }

                return std::make_shared<Values::UndefinedVal>();
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("input", Typechecker::g_strty, false) })))
        }
    },
    {
        "sleep",
        {
            std::make_shared<Values::NativeFnValue>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (!args.empty())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(args[0]->toNum())));
                }

                return std::make_shared<Values::UndefinedVal>();
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("milliseconds", Typechecker::g_numty, false) })))
        }
    },
    {
        "Regex",
        {
            std::make_shared<Values::NativeClassVal>([](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
            {
                if (args.empty()) throw ThrowException(ArgumentError("Usage: std::make_shared<Regex(expr: string)"));

                std::regex regex(args[0]->toString());
                std::shared_ptr<Values::ObjectVal> obj = std::make_shared<Values::ObjectVal>();

                obj->properties["match"] = std::make_shared<Values::NativeFnValue>([regex](std::vector<Values::Val> args, EnvPtr env) -> Values::Val
                {
                    if (args.empty()) throw ThrowException(ArgumentError("Usage: regex.match(input: string)"));

                    return std::make_shared<Values::BooleanVal>(std::regex_match(args[0]->toString(), regex));
                });

                return obj;
            }),
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>(
                {
                    {
                        "match",
                        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "native function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("input", Typechecker::g_strty, false) })))
                    }
                }
            )), "Regex")
        }
    }
};