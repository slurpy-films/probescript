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

    properties["join"] = std::make_shared<NativeFnValue>([this](std::vector<Val> args, EnvPtr env) -> Val
    {
        std::string seperator = (args.empty() ? "," : args[0]->toString());

        std::string result = "";
        for (size_t i = 0; i < this->items.size(); i++)
        {
            if (this->items.size() - 1 == i)
            {
                result += this->items[i]->toString();
            } else result += this->items[i]->toString() + seperator;
        }

        return std::make_shared<StringVal>(result);
    });
}