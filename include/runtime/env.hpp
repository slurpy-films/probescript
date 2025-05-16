#pragma once
#include <unordered_map>

#include <iostream>
#include "runtime/values.hpp"

Val evalCallWithFnVal(Val fn, std::vector<Val> args, Env* env);

class Env {
public:
    Env(Env* parentENV = nullptr);

    std::unordered_map<std::string, Val> variables;

    Val declareVar(std::string varName, Val value, bool constant = false);

    Val assignVar(std::string varName, Val value);

    Val lookupVar(std::string varName);

    Env* resolve(std::string varname);

    std::shared_ptr<ReturnSignal> throwErr(std::string err);

    void setCatch(Val fn);

    
    Val catcher;
    bool hasCatch = false;
private:
    Env* parent;
    std::unordered_map<std::string, bool> constants;
};


#include "stdlib/console.hpp"
#include "stdlib/datatypes.hpp"
#include "stdlib/process.hpp"