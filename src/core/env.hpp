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

extern std::unordered_map<std::string, std::pair<Val, TypePtr>> g_globals;

class Env;
using EnvPtr = std::shared_ptr<Env>;

#include "runtime/values.hpp"

Val evalCallWithFnVal(Val fn, std::vector<Val> args, EnvPtr env);

class Env : public std::enable_shared_from_this<Env>
{
public:
    Env(EnvPtr parentENV = nullptr);

    std::unordered_map<std::string, Val> variables = {};

    Val declareVar(std::string varName, Val value, Lexer::Token tk);

    Val assignVar(std::string varName, Val value, Lexer::Token tk);

    Val lookupVar(std::string varName, Lexer::Token tk);

    EnvPtr resolve(std::string varname, Lexer::Token tk);

private:
    EnvPtr parent;
    bool m_ready = false;

    void init();
};