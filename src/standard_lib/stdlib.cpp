#include "stdlib.hpp"

using namespace Probescript;
using namespace Probescript::Stdlib;

VM::ValuePtr getValRandomModule()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    return std::make_shared<VM::ObjectVal>(std::unordered_map<std::string, VM::ValuePtr>(
    {
        {
            "randint",
            std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext> ctx) -> VM::ValuePtr {
                if (args.size() < 2)
                {
                    throw ThrowException(ArgumentError("randInt expects two arguments"));
                }

                std::uniform_int_distribution<> distrib(args[0]->toNum(), args[1]->toNum());
                return std::make_shared<VM::NumberVal>(distrib(gen));
            })
        },
        {
            "rand",
            std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext> ctx) -> VM::ValuePtr {
                std::uniform_real_distribution<double> distrib(0.0, 1.0);
                return std::make_shared<VM::NumberVal>(distrib(gen));
            })
        }
    }));
};

std::unordered_map<std::string, Typechecker::TypePtr> g_typeStdlib =
{
    {
        "http",
        Http::getTypeHttpModule()
    },
    {
        "json",
        JSON::getTypeJsonModule()
    },
    {
        "random",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Module, "native module", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>(
        {
            {
                "randint",
                std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("x", Typechecker::g_numty, false), std::make_shared<Typechecker::Parameter>("y", Typechecker::g_numty, false) })))
            },
            {
                "rand",
                std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function")
            }
        })))
    },
    {
        "fs",
        Fs::getTypeFsModule()
    },
    {
        "date",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Module, "native module", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>(
        {
            {
                "stamp",
                std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("format", Typechecker::g_strty, false) })))
            },
            {
                "now",
                std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function")
            },
            {
                "is_leapyear",
                std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("year", Typechecker::g_numty, false) })))
            },
            {
                "days_in_month",
                std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("year", Typechecker::g_numty), std::make_shared<Typechecker::Parameter>("month", Typechecker::g_numty) })))
            }
        })))
    },
    {
        "prbtest",
        Prbtest::getTypeTestLib()
    }
};

std::unordered_map<std::string, VM::ValuePtr> g_valStdlib =
{
    {
        "http",
        Http::getValHttpModule(),
    },
    {
        "json",
        JSON::getValJsonModule(),
    },
    {
        "random",
        getValRandomModule(),
    },
    {
        "fs",
        Fs::getValFsModule(),
    },
    {
        "date",
        std::make_shared<VM::ObjectVal>(std::unordered_map<std::string, VM::ValuePtr>(
        {
            {
                "stamp",
                std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext> ctx) -> VM::ValuePtr
                {
                    using namespace std::chrono;

                    auto now = system_clock::now();
                    auto duration = now.time_since_epoch();

                    std::string unit = "sec";
                    if (!args.empty() && args[0]->type == VM::ValueType::String)
                    {
                        unit = std::static_pointer_cast<VM::StringVal>(args[0])->string;
                        std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
                    }

                    long long result;

                    if (unit == "milli")
                    {
                        result = duration_cast<milliseconds>(duration).count();
                    }
                    else if (unit == "sec")
                    {
                        result = duration_cast<seconds>(duration).count();
                    }
                    else if (unit == "min")
                    {
                        result = duration_cast<minutes>(duration).count();
                    }
                    else if (unit == "hour")
                    {
                        result = duration_cast<hours>(duration).count();
                    }
                    else
                    {
                        throw ThrowException("Invalid time unit: " + unit);
                    }
                    return std::make_shared<VM::NumberVal>((double)result);
                })
            },
            {
                "now",
                std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext> ctx) -> VM::ValuePtr
                {
                    using namespace std::chrono;
                    auto now = system_clock::now();
                    auto t = system_clock::to_time_t(now);
                    std::ostringstream oss;
                    oss << std::put_time(std::localtime(&t), "%Y-%m-%dT%H:%M:%S");
                    return std::make_shared<VM::StringVal>(oss.str());
                })
            },
            {
                "is_leapyear",
                std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext> ctx) -> VM::ValuePtr {
                    if (args.empty() || args[0]->type != VM::ValueType::Number)
                        throw ThrowException(ArgumentError("Usage: date.is_leapyear(year: num)"));

                    int year = static_cast<int>(std::static_pointer_cast<VM::NumberVal>(args[0])->number);

                    bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
                    return std::make_shared<VM::BooleanVal>(leap);
                })
            },
            {
                "days_in_month",
                std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext> ctx) -> VM::ValuePtr {
                    if (args.size() < 2 || args[0]->type != VM::ValueType::Number || args[1]->type != VM::ValueType::Number)
                        throw ThrowException(ArgumentError("Usage: date.days_in_month(year: num, month: num)"));

                    int year = static_cast<int>(std::static_pointer_cast<VM::NumberVal>(args[0])->number);
                    int month = static_cast<int>(std::static_pointer_cast<VM::NumberVal>(args[1])->number);

                    if (month < 1 || month > 12)
                        throw ThrowException(CustomError("Invalid month: " + std::to_string(month), "DateError"));

                    int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
                    int result = days[month - 1];

                    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
                        result = 29;

                    return std::make_shared<VM::NumberVal>((double)result);
                })
            }
        }))
    },
    {
        "prbtest",
        Prbtest::getValTestLib()
    }
};

// This will be removed when the lagacy interpreter is removed
std::unordered_map<std::string, std::pair<Probescript::Values::Val, Probescript::Typechecker::TypePtr>> g_stdlib = {};