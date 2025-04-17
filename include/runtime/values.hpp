#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "../ast.hpp"

struct Env;

namespace ValueType {
    enum ValueType {
        Null,
        Number,
        Boolean,
        Object,
        NativeFn,
        Function,
        String,
    };
}


struct RuntimeVal {
    ValueType::ValueType type;
    std::string value;

    RuntimeVal(ValueType::ValueType type) : type(type), value("") {}
    RuntimeVal(ValueType::ValueType type, const std::string& val) : type(type), value(val) {}
    virtual ~RuntimeVal() = default;
};


struct NullVal : public RuntimeVal {
    NullVal() : RuntimeVal(ValueType::Null, "null") {}
};

struct NumberVal : public RuntimeVal {
    std::string number;

    NumberVal(std::string val) : RuntimeVal(ValueType::Number, val), number(val) {}
};

struct StringVal : public RuntimeVal {
    std::string string;

    StringVal(std::string val) : RuntimeVal(ValueType::String, val), string(val) {}
};

struct BooleanVal : public RuntimeVal {
    std::string value;

    BooleanVal(std::string val) : RuntimeVal(ValueType::Boolean, val), value(val) {}

    bool getValue() {
        return value == "true";
    }
};

struct ObjectVal : public RuntimeVal {
    ObjectVal(std::unordered_map<std::string, RuntimeVal*> properties = {})
        : RuntimeVal(ValueType::Object), properties(properties) {}

    std::unordered_map<std::string, RuntimeVal*> properties;
};


using NativeFunction = std::function<RuntimeVal*(std::vector<RuntimeVal*>, Env*)>;

struct NativeFnValue : public RuntimeVal {
    NativeFnValue(NativeFunction fn) 
    : RuntimeVal(ValueType::NativeFn), call(fn) {}
    NativeFunction call;
};

struct FunctionValue : public RuntimeVal {
    FunctionValue (string name, vector<string> params, Env* declarationEnv, vector<Stmt*> body) 
    : RuntimeVal(ValueType::Function), name(name), params(params), declarationEnv(declarationEnv), body(body) {}
    string name;
    vector<string> params;
    Env* declarationEnv;
    vector<Stmt*> body;
};