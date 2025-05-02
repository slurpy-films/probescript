#pragma once
#include <string>
#include "runtime/env.hpp"
#include "runtime/values.hpp"


using NativeFunction = std::function<RuntimeVal*(std::vector<RuntimeVal*>, Env*)>;

std::unordered_map<std::string, RuntimeVal*> console = {
    { "println", new NativeFnValue([](std::vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
        for (RuntimeVal* arg : args) {
            std::cout << arg->toString() << " ";
        }

        std::cout << std::endl;
        return new NullVal();
    }) },
    {"prompt", new NativeFnValue([](std::vector<RuntimeVal*> args, Env* env) -> RuntimeVal* {
        for (RuntimeVal* arg : args) {
            switch (arg->type) {
                case ValueType::Number:
                    std::cout << static_cast<NumberVal*>(arg)->number;
                    break;

                case ValueType::String:
                std::cout << static_cast<StringVal*>(arg)->string;
                    break;
                case ValueType::Boolean: {
                    bool booleanval = (static_cast<BooleanVal*>(arg)->getValue());
                    if (booleanval) {
                        std::cout << "true";
                    } else {
                        std::cout << "false";
                    }

                    break;
                }
                case ValueType::Object: {
                    ObjectVal* obj = static_cast<ObjectVal*>(arg);

                    for (const auto& pair : obj->properties) {
                        std::cout << pair.first << "\n";
                    }
                    break;
                }

                default:
                std::cout << "Invalid type: " << arg->type;
                    exit(1);
            }
        }

        std::string input;
        std::getline(std::cin, input);

        return new StringVal(input);
    })
}};