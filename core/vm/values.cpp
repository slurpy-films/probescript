#include "values.hpp"

using namespace Probescript::VM;

ValuePtr Scope::declareVar(std::string name, ValuePtr val)
{
    if (!m_ready) init();

    m_variables[name] = val;
    return val;
}

ValuePtr Scope::lookupVar(std::string varName)
{
    if (!m_ready) init();
    
    return resolve(varName)->m_variables[varName];
}

ScopePtr Scope::resolve(std::string varName)
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

void Scope::init()
{
    m_ready = true;
    
    declareVar("print", std::make_shared<NativeFunctionVal>([](std::vector<ValuePtr> args) -> ValuePtr
    {
        for (auto& arg : args)
        {
            std::cout << arg->toString();
        }

        std::cout << '\n';
        return std::make_shared<NullVal>();
    }));
}