#pragma once
#include <unordered_map>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "types.hpp"
#include "runtime/values.hpp"
#include "utils.hpp"
#include "frontend/lexer.hpp"
#include "errors.hpp"

extern std::unordered_map<std::string, std::pair<Probescript::Values::Val, Probescript::Typechecker::TypePtr>> g_globals;

namespace Probescript
{

class Env;
using EnvPtr = std::shared_ptr<Env>;

} // namespace Probescript

#include "runtime/values.hpp"

namespace Probescript
{

class Env : public std::enable_shared_from_this<Env>
{
public:
    Env(EnvPtr parentENV = nullptr);

    std::unordered_map<std::string, Values::Val> variables = {};

    Values::Val declareVar(std::string varName, Values::Val value, Lexer::Token tk);

    Values::Val assignVar(std::string varName, Values::Val value, Lexer::Token tk);

    Values::Val lookupVar(std::string varName, Lexer::Token tk);

    EnvPtr resolve(std::string varname, Lexer::Token tk);
private:
    EnvPtr parent;
    bool m_ready = false;

    void init();
};

} // namespace Probescript