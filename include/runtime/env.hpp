#pragma once
#include <unordered_map>
#include "values.hpp"
#include "stdlib/console.hpp"
#include "stdlib/datatypes.hpp"
#include <iostream>


class Env {
    public:
        Env(Env* parentENV = nullptr) {
            bool global = (parentENV == nullptr);
            parent = parentENV;

            if (global) {
                declareVar("true", new BooleanVal("true"), true);
                declareVar("false", new BooleanVal("false"), true);
                declareVar("num", new NativeFnValue(toNum), true);
                declareVar("str", new NativeFnValue(toStr), true);
                declareVar("console", new ObjectVal(console), true);
            }
        }

        std::unordered_map<std::string, RuntimeVal*> variables;

        RuntimeVal* declareVar(std::string varName, RuntimeVal* value, bool constant = false) {
            if (variables.find(varName) != variables.end()) {
                std::cout << "Variable " << varName << " is already defined" << std::endl;
                exit(1);
            }

            variables[varName] = value;
            constants[varName] = constant;
            return value;
        }

        RuntimeVal* assignVar(std::string varName, RuntimeVal* value) {
            Env* env = resolve(varName);

            if (env->constants.find(varName) != env->constants.end() && env->constants[varName]) {
                std::cout << "Assignment to constant variable " << varName << std::endl;
                exit(1);
            }

            env->variables[varName] = value;

            return value;
        }

        RuntimeVal* lookupVar(std::string varName) {
            Env* env = resolve(varName);
            return env->variables[varName];
        }

        Env* resolve(std::string varname) {
            if (variables.find(varname) != variables.end()) {
                return this;
            }

            if (parent == nullptr) {
                std::cout << "Cannot resolve variable " << varname << " as it does not exist";
                exit(1);
            }

            return parent->resolve(varname);
        }

    private:
        Env* parent;
        std::unordered_map<std::string, bool> constants;
};