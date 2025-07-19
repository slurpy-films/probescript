#include "env.hpp"

using namespace Probescript;

Env::Env(EnvPtr parentENV)
{
    parent = parentENV;
}

void Env::init()
{
    m_ready = true;
    for (const auto& [key, pair] : g_globals)
        declareVar(key, pair.first, Lexer::Token());
}

Values::Val Env::declareVar(std::string varname, Values::Val value, Lexer::Token tk)
{
    if (!m_ready) init();
    if (variables.find(varname) != variables.end())
    {
        throw std::runtime_error(CustomError("Variable " + varname + " is already defined", "ReferenceError", tk));
    }

    variables[varname] = value;
    return value;
}

Values::Val Env::assignVar(std::string varname, Values::Val value, Lexer::Token tk)
{
    if (!m_ready) init();
    EnvPtr env = resolve(varname, tk);

    env->variables[varname] = value;

    return value;
}

Values::Val Env::lookupVar(std::string varname, Lexer::Token tk)
{
    if (!m_ready) init();
    EnvPtr env = resolve(varname, tk);

    return env->variables[varname];
}

EnvPtr Env::resolve(std::string varname, Lexer::Token tk)
{
    if (!m_ready) init();

    if (variables.find(varname) != variables.end())
    {
        return shared_from_this();
    }

    if (!parent)
    {
        throw std::runtime_error(CustomError("Cannot resolve variable " + varname + " as it does not exist", "ReferenceError", tk));
    }

    return parent->resolve(varname, tk);
}