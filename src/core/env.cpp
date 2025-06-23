#include "env.hpp"

Env::Env(EnvPtr parentENV)
{
    parent = parentENV;
}

void Env::init()
{
    m_ready = true;
    for (const auto& [key, pair] : g_globals)
        declareVar(key, pair.first);
}

Val Env::declareVar(std::string varname, Val value, bool constant)
{
    if (!m_ready) init();
    if (variables.find(varname) != variables.end())
    {
        throw std::runtime_error("Variable " + varname + " is already defined\n");
    }

    variables[varname] = value;
    constants[varname] = constant;
    return value;
}

Val Env::assignVar(std::string varname, Val value)
{
    if (!m_ready) init();
    EnvPtr env = resolve(varname);

    if (env->constants.find(varname) != env->constants.end() && env->constants[varname])
    {
        throw std::runtime_error("Assignment to constant variable " + varname + "\n");
    }

    env->variables[varname] = value;

    return value;
}

Val Env::lookupVar(std::string varname)
{
    if (!m_ready) init();
    EnvPtr env = resolve(varname);

    return env->variables[varname];
}

std::shared_ptr<ReturnSignal> Env::throwErr(std::string err)
{
    if (!m_ready) init();
    if (hasCatch)
    {
        evalCallWithFnVal(catcher, { std::make_shared<StringVal>(err) }, shared_from_this());
    } else if (parent)
    {
        return parent->throwErr(err);
    } else
    {
        throw std::runtime_error(err);
    }

    return std::make_shared<ReturnSignal>(std::make_shared<UndefinedVal>());
}

EnvPtr Env::resolve(std::string varname)
{
    if (!m_ready) init();

    if (variables.find(varname) != variables.end())
    {
        return shared_from_this();
    }

    if (!parent)
    {
        throw std::runtime_error("Cannot resolve variable " + varname + " as it does not exist");
    }

    return parent->resolve(varname);
}

void Env::setCatch(Val fn)
{
    if (!m_ready) init();
    hasCatch = true;
    catcher = fn;
}