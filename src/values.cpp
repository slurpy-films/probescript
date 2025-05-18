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