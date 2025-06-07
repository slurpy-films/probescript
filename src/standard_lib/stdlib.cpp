#include "stdlib.hpp"

Val getValRandomModule() {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    return std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>({
        {
            "randInt",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2) {
                    return env->throwErr(ArgumentError("randInt expects two arguments"));
                }

                std::uniform_int_distribution<> distrib(args[0]->toNum(), args[1]->toNum());
                return std::make_shared<NumberVal>(distrib(gen));
            })
        },
        {
            "rand",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                std::uniform_real_distribution<double> distrib(0.0, 1.0);
                return std::make_shared<NumberVal>(distrib(gen));
            })
        }
    }));
};

std::unordered_map<std::string, std::pair<Val, TypePtr>> g_stdlib =
{
    {
        "http",
        {
            getValHttpModule(),
            getTypeHttpModule()
        }
    },
    {
        "json",
        {
            getValJsonModule(),
            getTypeJsonModule()
        }
    },
    {
        "random",
        {
            getValRandomModule(),
            std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
                {
                    "randInt",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "x"), new VarDeclarationType(new UndefinedLiteralType(), "y") })))
                },
                {
                    "rand",
                    std::make_shared<Type>(TypeKind::Function, "native function")
                }
            })))
        }
    },
    {
        "fs",
        {
            getValFsModule(),
            getTypeFsModule()
        }
    },
    {
        "date",
        {
            std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>({
                {
                    "stamp",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
                    {
                        using namespace std::chrono;

                        auto now = system_clock::now();
                        auto duration = now.time_since_epoch();

                        std::string unit = "sec";
                        if (!args.empty() && args[0]->type == ValueType::String)
                        {
                            unit = std::static_pointer_cast<StringVal>(args[0])->string;
                            std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
                        }

                        long long result;

                        if (unit == "milli")
                        {
                            result = duration_cast<milliseconds>(duration).count();
                        } else if (unit == "sec")
                        {
                            result = duration_cast<seconds>(duration).count();
                        } else if (unit == "min")
                        {
                            result = duration_cast<minutes>(duration).count();
                        } else if (unit == "hour")
                        {
                            result = duration_cast<hours>(duration).count();
                        } else
                        {
                            return std::make_shared<StringVal>("Invalid time unit: " + unit);
                        }
                        return std::make_shared<NumberVal>(std::to_string(result));
                    })
                }
            })),
            std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
                {
                    "stamp",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "format", new IdentifierType("str")) })))
                }
            })))
        }
    }
};