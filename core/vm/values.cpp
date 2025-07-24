#include "values.hpp"

using namespace Probescript::VM;

ValuePtr Scope::declare(const std::string& name, ValuePtr val)
{
    if (!m_ready) init();

    m_variables[name] = val;
    return val;
}

ValuePtr Scope::assign(const std::string& name, ValuePtr val)
{
    if (!m_ready) init();

    auto scope = resolve(name);
    scope->m_variables[name] = val;
    return val;
}

ValuePtr Scope::lookupVar(const std::string& varName)
{
    if (!m_ready) init();
    
    return resolve(varName)->m_variables[varName];
}

ScopePtr Scope::resolve(const std::string& varName)
{
    if (!m_ready) init();
    
    if (m_variables.find(varName) != m_variables.end())
    {
        return shared_from_this();
    }
    else if (m_parent)
    {
        return m_parent->resolve(varName);
    }
    else
    {
        throw std::runtime_error("Cannot resolve variable " + varName + " as it does not exist");
    }
}

ScopePtr Scope::getParent()
{
    return m_parent;
}

void Scope::init()
{
    m_ready = true;
    
    declare("print", std::make_shared<NativeFunctionVal>([](std::vector<ValuePtr> args) -> ValuePtr
    {
        for (auto& arg : args)
        {
            if (arg->type == ValueType::Number)
            {
                std::cout << arg->toNum();
            }
            else
            {
                std::cout << arg->toString();
            }
        }

        std::cout << '\n';
        return std::make_shared<NullVal>();
    }));
}

ValuePtr Value::add(const ValuePtr o) const
{
    return std::make_shared<NullVal>();
}

ValuePtr Value::sub(const ValuePtr o) const
{
    return std::make_shared<NullVal>();
}

bool Value::compare(const ValuePtr o) const
{
    return false;
}