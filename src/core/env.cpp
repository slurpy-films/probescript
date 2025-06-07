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
        std::cout << "Variable " << varname << " is already defined" << std::endl;
        exit(1);
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
        std::cout << "Assignment to constant variable " << varname << std::endl;
        exit(1);
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
        std::cerr << err;
        exit(1);
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
        std::cout << "Cannot resolve variable " << varname << " as it does not exist";
        exit(1);
    }

    return parent->resolve(varname);
}

void Env::setCatch(Val fn)
{
    if (!m_ready) init();
    hasCatch = true;
    catcher = fn;
}