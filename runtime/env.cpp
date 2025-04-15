#pragma once
#include <unordered_map>
#include "values.hpp"
#include <iostream>

using namespace std;

class Env {
    public:
        Env(Env* parentENV = nullptr) {
            bool global = (parentENV == nullptr);
            parent = parentENV;

            if (global) {
                declareVar("true", new BooleanVal("true"), true);
                declareVar("false", new BooleanVal("false"), true);

                unordered_map<string, RuntimeVal*> console = {
                    { "log", new NativeFnValue([](vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
                        for (auto* arg : args) {
                            switch (arg->type) {
                                case ValueType::Number:
                                    cout << static_cast<NumberVal*>(arg)->number;
                                    break;
                
                                default:
                                    cout << "Invalid type: " << arg->type;
                                    exit(1);
                            }
                        }
                
                        cout << endl;
                        return new NullVal();
                    }) }
                };

                declareVar("console", new ObjectVal(console));
            }
        }

        RuntimeVal* declareVar(string varName, RuntimeVal* value, bool constant = false) {
            if (variables.find(varName) != variables.end()) {
                cout << "Variable " << varName << " is already defined" << endl;
                exit(1);
            }

            variables[varName] = value;
            constants[varName] = constant;
            return value;
        }

        RuntimeVal* assignVar(string varName, RuntimeVal* value) {
            Env* env = resolve(varName);

            if (env->constants.find(varName) != env->constants.end() && env->constants[varName]) {
                cout << "Assignment to constant variable " << varName << endl;
                exit(1);
            }

            env->variables[varName] = value;

            return value;
        }

        RuntimeVal* lookupVar(string varName) {
            Env* env = resolve(varName);
            return env->variables[varName];
        }

        Env* resolve(string varname) {
            if (variables.find(varname) != variables.end()) {
                return this;
            }

            if (parent == nullptr) {
                cout << "Cannot resolve variable " << varname << " as it does not exist";
                exit(1);
            }

            return parent->resolve(varname);
        }

    private:
        Env* parent;
        unordered_map<string, RuntimeVal*> variables;
        unordered_map<string, bool> constants;
};