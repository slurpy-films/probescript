#pragma once
#include <unordered_map>
#include <iostream>
#include <memory>
#include "types.hpp"
#include "runtime/values.hpp"

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

    Val declareVar(std::string varName, Val value, bool constant = false);

    Val assignVar(std::string varName, Val value);

    Val lookupVar(std::string varName);

    EnvPtr resolve(std::string varname);

    std::shared_ptr<ReturnSignal> throwErr(std::string err);

    void setCatch(Val fn);

    Val catcher;
    bool hasCatch = false;
private:
    EnvPtr parent;
    std::unordered_map<std::string, bool> constants;
    bool m_ready = false;

    void init();
};