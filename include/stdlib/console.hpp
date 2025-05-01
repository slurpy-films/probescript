#pragma once
#include <string>
#include "runtime/env.hpp"
#include "runtime/values.hpp"

using namespace std;

unordered_map<string, RuntimeVal*> console = {
    { "println", new NativeFnValue([](vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
        for (auto* arg : args) {
            switch (arg->type) {
                case ValueType::Number:
                    cout << static_cast<NumberVal*>(arg)->number;
                    break;

                case ValueType::String:
                    cout << static_cast<StringVal*>(arg)->string;
                    break;
                case ValueType::Boolean: {
                    bool booleanval = (static_cast<BooleanVal*>(arg)->getValue());
                    if (booleanval) {
                        cout << "true";
                    } else {
                        cout << "false";
                    }

                    break;
                }
                case ValueType::Object: {
                    ObjectVal* obj = static_cast<ObjectVal*>(arg);

                    for (const auto& pair : obj->properties) {
                        cout << pair.first << "\n";
                    }
                    break;
                }

                default:
                    cout << "Invalid type: " << arg->type;
                    exit(1);
            }
            cout << " ";
        }

        cout << endl;
        return new NullVal();
    }) },
    {"prompt", new NativeFnValue([](vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
        for (RuntimeVal* arg : args) {
            switch (arg->type) {
                case ValueType::Number:
                    cout << static_cast<NumberVal*>(arg)->number;
                    break;

                case ValueType::String:
                    cout << static_cast<StringVal*>(arg)->string;
                    break;
                case ValueType::Boolean: {
                    bool booleanval = (static_cast<BooleanVal*>(arg)->getValue());
                    if (booleanval) {
                        cout << "true";
                    } else {
                        cout << "false";
                    }

                    break;
                }
                case ValueType::Object: {
                    ObjectVal* obj = static_cast<ObjectVal*>(arg);

                    for (const auto& pair : obj->properties) {
                        cout << pair.first << "\n";
                    }
                    break;
                }

                default:
                    cout << "Invalid type: " << arg->type;
                    exit(1);
            }
        }

        string input;
        getline(cin, input);

        return new StringVal(input);
    })
}};