#include "stdlib.hpp"

Val getValRandomModule() {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    return std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>({
        {
            "randint",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.size() < 2) {
                    throw ThrowException(ArgumentError("randInt expects two arguments"));
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
                    "randint",
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
                },
                {
                    "now",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
                    {
                        using namespace std::chrono;
                        auto now = system_clock::now();
                        auto t = system_clock::to_time_t(now);
                        std::ostringstream oss;
                        oss << std::put_time(std::localtime(&t), "%Y-%m-%dT%H:%M:%S");
                        return std::make_shared<StringVal>(oss.str());
                    })
                },
                {
                    "is_leapyear",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                        if (args.empty() || args[0]->type != ValueType::Number)
                            throw ThrowException(ArgumentError("Usage: date.is_leapyear(year: num)"));

                        int year = static_cast<int>(std::static_pointer_cast<NumberVal>(args[0])->number);

                        bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
                        return std::make_shared<BooleanVal>(leap);
                    })
                },
                {
                    "days_in_month",
                    std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val {
                        if (args.size() < 2 || args[0]->type != ValueType::Number || args[1]->type != ValueType::Number)
                            throw ThrowException(ArgumentError("Usage: date.days_in_month(year: num, month: num)"));

                        int year = static_cast<int>(std::static_pointer_cast<NumberVal>(args[0])->number);
                        int month = static_cast<int>(std::static_pointer_cast<NumberVal>(args[1])->number);

                        if (month < 1 || month > 12)
                            throw ThrowException(CustomError("Invalid month: " + std::to_string(month), "DateError"));

                        int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
                        int result = days[month - 1];

                        if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
                            result = 29;

                        return std::make_shared<NumberVal>(std::to_string(result));
                    })
                }
            })),
            std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>({
                {
                    "stamp",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "format", new IdentifierType("str")) })))
                },
                {
                    "now",
                    std::make_shared<Type>(TypeKind::Function, "native function")
                },
                {
                    "is_leapyear",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "year", new IdentifierType("num")) })))
                },
                {
                    "days_in_month",
                    std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "year", new IdentifierType("num")), new VarDeclarationType(new UndefinedLiteralType(), "month", new IdentifierType("num")) })))
                }
            })))
        }
    },
    {
        "prbtest",
        {
            getValTestLib(),
            getTypeTestLib()
        }
    }
};