#include "runtime/values.hpp"


Val RuntimeVal::add(Val o) const
{
    return std::make_shared<NumberVal>(0 + o->toNum());
}

Val RuntimeVal::sub(Val o) const
{
    return std::make_shared<NumberVal>(0 - o->toNum()); 
}

Val RuntimeVal::mul(Val o) const
{
    return std::make_shared<NumberVal>(0);
}

Val RuntimeVal::div(Val o) const
{
    return std::make_shared<NumberVal>(0);
}

Val RuntimeVal::mod(Val o) const
{
    return std::make_shared<NumberVal>(0);
}

ArrayVal::ArrayVal(std::vector<Val> items) : RuntimeVal(ValueType::Array), items(items)
{
    properties["size"] = std::make_shared<NativeFnValue>([this](std::vector<Val> _args, EnvPtr _env) -> Val
    {
        return std::make_shared<NumberVal>(this->items.size());
    });

    properties["push"] = std::make_shared<NativeFnValue>([this](std::vector<Val> args, EnvPtr _env) -> Val
    {
        for (Val arg : args)
        {
            this->items.push_back(arg);
        }

        return std::make_shared<UndefinedVal>();
    });

    properties["join"] = std::make_shared<NativeFnValue>([items](std::vector<Val> args, EnvPtr env) -> Val
    {
        std::string separator = args.empty() ? "," : args[0]->toString();
        std::string result;

        for (size_t i = 0; i < items.size(); ++i)
        {

            result += items[i]->toString();

            if (i != items.size() - 1)
            {
                result += separator;
            }
        }

        return std::make_shared<StringVal>(result);
    });

    properties["foreach"] = std::make_shared<NativeFnValue>([items](std::vector<Val> args, EnvPtr env) -> Val
    {
        if (args.empty())
            return std::make_shared<UndefinedVal>();

        for (size_t i = 0; i < items.size(); ++i)
            evalCallWithFnVal(args[0], { items[i] }, env);

        return std::make_shared<UndefinedVal>();
    });

    properties["map"] = std::make_shared<NativeFnValue>([items](std::vector<Val> args, EnvPtr env) -> Val
    {
        if (args.empty())
            return std::make_shared<UndefinedVal>();

        auto result = std::make_shared<ArrayVal>();

        for (size_t i = 0; i < items.size(); ++i)
            result->items.push_back(evalCallWithFnVal(args[0], { items[i], std::make_shared<NumberVal>(i) }, env));

        return result;
    });

    properties["filter"] = std::make_shared<NativeFnValue>([items](std::vector<Val> args, EnvPtr env) -> Val
    {
        if (args.empty())
            return std::make_shared<UndefinedVal>();

        auto result = std::make_shared<ArrayVal>();

        for (size_t i = 0; i < items.size(); ++i)
            if (evalCallWithFnVal(args[0], { items[i], std::make_shared<NumberVal>(i) }, env)->toBool())
                result->items.push_back(items[i]);

        return result;
    });
}

StringVal::StringVal(std::string val) : RuntimeVal(ValueType::String), string(val)
{
    properties = 
    {
        {
            "length",
            std::make_shared<NativeFnValue>([this](std::vector<Val> _args, EnvPtr _env) -> Val
            {
                return std::make_shared<NumberVal>(this->string.length());
            })
        },
        {
            "split",
            std::make_shared<NativeFnValue>([this](std::vector<Val> args, EnvPtr _) -> Val
            {
                if (args.empty() || args[0]->type != ValueType::String)
                    return std::make_shared<UndefinedVal>();

                std::string str = this->string;
                std::string deli = std::static_pointer_cast<StringVal>(args[0])->string;

                std::vector<Val> result;
                std::vector<std::string> items = split(str, deli);
                for (const std::string item : items)
                    result.push_back(std::make_shared<StringVal>(item));

                return std::make_shared<ArrayVal>(result);
            })
        },
        {
            "tolower",
            std::make_shared<NativeFnValue>([this](std::vector<Val> _args, EnvPtr _env) -> Val
            {
                std::string temp = this->string;
                std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c){ return std::tolower(c); });
                return std::make_shared<StringVal>(temp);
            })
        },
        {
            "toupper",
            std::make_shared<NativeFnValue>([this](std::vector<Val> _args, EnvPtr _env) -> Val
            {
                std::string temp = this->string;
                std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c){ return std::toupper(c); });
                return std::make_shared<StringVal>(temp);
            })
        },
        {
            "startswith",
            std::make_shared<NativeFnValue>([this](std::vector<Val> args, EnvPtr _env) -> Val
            {
                return std::make_shared<BooleanVal>(!args.empty() && (this->string.find(args[0]->toString()) == 0));
            })
        },
        {
            "find",
            std::make_shared<NativeFnValue>([this](std::vector<Val> args, EnvPtr _env) -> Val
            {
                return std::make_shared<NumberVal>(!args.empty() ? this->string.find(args[0]->toString()) : -1);
            })
        },
        {
            "includes",
            std::make_shared<NativeFnValue>([this](std::vector<Val> args, EnvPtr _env) -> Val
            {
                return std::make_shared<BooleanVal>(!args.empty() && this->string.find(args[0]->toString()) != std::string::npos);
            })
        }
    };
}

ReturnSignal::ReturnSignal(Val val, std::string msg)
    : m_val(val), std::runtime_error(msg) {}

Val ReturnSignal::get() const
{
    return m_val;
}

BreakSignal::BreakSignal(std::string msg)
    : std::runtime_error(msg) {};

ContinueSignal::ContinueSignal(std::string msg)
    : std::runtime_error(msg) {};