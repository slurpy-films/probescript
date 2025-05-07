#pragma once
#include <unordered_map>
#include "values.hpp"
#include "stdlib/console.hpp"
#include "stdlib/datatypes.hpp"
#include "stdlib/process.hpp"
#include <iostream>


class Env {
    public:
        Env(Env* parentENV = nullptr) {
            bool global = (parentENV == nullptr);
            parent = parentENV;

            if (global) {
                declareVar("true", std::make_shared<BooleanVal>(true), true);
                declareVar("false", std::make_shared<BooleanVal>(false), true);
                declareVar("num", std::make_shared<NativeFnValue>(toNum), true);
                declareVar("str", std::make_shared<NativeFnValue>(toStr), true);
                declareVar("console", std::make_shared<ObjectVal>(getConsole()), true);
                declareVar("process", std::make_shared<ObjectVal>(getProcessModule()));
                declareVar("undefined", std::make_shared<UndefinedVal>(), true);
            }
        }

        std::unordered_map<std::string, Val> variables;

        Val declareVar(std::string varName, Val value, bool constant = false) {
            if (variables.find(varName) != variables.end()) {
                std::cout << "Variable " << varName << " is already defined" << std::endl;
                exit(1);
            }

            variables[varName] = value;
            constants[varName] = constant;
            return value;
        }

        Val assignVar(std::string varName, Val value) {
            Env* env = resolve(varName);

            if (env->constants.find(varName) != env->constants.end() && env->constants[varName]) {
                std::cout << "Assignment to constant variable " << varName << std::endl;
                exit(1);
            }

            env->variables[varName] = value;

            return value;
        }

        Val lookupVar(std::string varName) {
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