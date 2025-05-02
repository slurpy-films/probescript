#pragma once
#include <string>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <functional>
#include "../ast.hpp"

struct Env;

namespace ValueType {
    enum ValueType {
        Probe,
        Null,
        Number,
        Boolean,
        Object,
        NativeFn,
        Function,
        String,
        Undefined,
        Array,
        Class,
    };
}



struct RuntimeVal {
    ValueType::ValueType type;
    std::string value;
    std::unordered_map<std::string, RuntimeVal*> exports;

    RuntimeVal(ValueType::ValueType type) : type(type), value("") {}
    RuntimeVal(ValueType::ValueType type, const std::string& val) : type(type), value(val) {}
    virtual ~RuntimeVal() = default;

    virtual std::string toString() const {
        return "[object RuntimeVal]";
    }

    virtual double toNum() const {
        return 0;
    }
};

using NativeFunction = std::function<RuntimeVal*(std::vector<RuntimeVal*>, Env*)>;


struct UndefinedVal : public RuntimeVal {
    UndefinedVal() : RuntimeVal(ValueType::Undefined, "undefined") {}
    std::string toString() const override { return "undefined"; }

    double toNum() const override { return 0; }
};

struct NullVal : public RuntimeVal {
    NullVal() : RuntimeVal(ValueType::Null, "null") {}
    std::string toString() const override { return "null"; }
    double toNum() const override { return 0; }
};

struct NumberVal : public RuntimeVal {
    std::string number;
    NumberVal(std::string val) : RuntimeVal(ValueType::Number, val), number(val) {}
    double getValue() { return stod(number); }
    std::string toString() const override {
        double num = stod(number);
        return (std::floor(num) == num) ? std::to_string(static_cast<int>(num)) : std::to_string(num);
    }
    double toNum() const override { return stod(number); }
};

struct StringVal : public RuntimeVal {
    std::string string;
    StringVal(std::string val) : RuntimeVal(ValueType::String, val), string(val) {}
    std::string toString() const override { return string; }
    double toNum() const override { return stod(string); }
};

struct BooleanVal : public RuntimeVal {
    std::string value;
    BooleanVal(std::string val) : RuntimeVal(ValueType::Boolean, val), value(val) {}
    bool getValue() { return value == "true" || value == "1"; }
    std::string toString() const override { return value; }
    double toNum() const override { return value == "true" || value == "1" ? 1 : 0; }
};

struct ObjectVal : public RuntimeVal {
    std::unordered_map<std::string, RuntimeVal*> properties;
    ObjectVal(std::unordered_map<std::string, RuntimeVal*> properties = {})
        : RuntimeVal(ValueType::Object), properties(properties) {}
    std::string toString() const override {
        std::string result = "{ ";
        bool first = true;
        for (const auto& [key, val] : properties) {
            if (!first) result += ", ";
            result += key + ": " + val->toString();
            first = false;
        }
        result += " }";
        return result;
    }
};

struct ArrayVal : public RuntimeVal {
    std::vector<RuntimeVal*> items;
    ArrayVal(std::vector<RuntimeVal*> items) : RuntimeVal(ValueType::Array), items(items) {}
    std::string toString() const override {
        std::string result = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            result += items[i]->toString();
            if (i < items.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
};

struct NativeFnValue : public RuntimeVal {
    NativeFunction call;
    NativeFnValue(NativeFunction fn) : RuntimeVal(ValueType::NativeFn), call(fn) {}
    std::string toString() const override {
        return "[native function]";
    }
};

struct FunctionValue : public RuntimeVal {
    std::string name;
    std::vector<std::string> params;
    Env* declarationEnv;
    std::vector<Stmt*> body;
    FunctionValue (std::string name, std::vector<std::string> params, Env* declarationEnv, std::vector<Stmt*> body) 
        : RuntimeVal(ValueType::Function), name(name), params(params), declarationEnv(declarationEnv), body(body) {}
    std::string toString() const override {
        return "[function " + name + "]";
    }
};

struct ProbeValue : public RuntimeVal {
    std::string name;
    Env* declarationEnv;
    std::vector<Stmt*> body;
    ProbeValue (std::string name, Env* declarationEnv, std::vector<Stmt*> body) 
        : RuntimeVal(ValueType::Probe), name(name), declarationEnv(declarationEnv), body(body) {}
    std::string toString() const override {
        return "[probe " + name + "]";
    }
};

struct ClassVal : public RuntimeVal {
    std::string name;
    Env* parentEnv;
    std::vector<Stmt*> body;
    Expr* extends;
    bool doesExtend = false;
    ClassVal(std::string name, Env* declarationEnv, std::vector<Stmt*> body) 
    : RuntimeVal(ValueType::Class), name(name), parentEnv(declarationEnv), body(body) {}
    ClassVal(std::string name, Env* declarationEnv, std::vector<Stmt*> body, Expr* extends) 
    : RuntimeVal(ValueType::Class), name(name), parentEnv(declarationEnv), body(body), extends(extends), doesExtend(true) {}
    std::string toString() const override {
        return "[class " + name + "]";
    }
};