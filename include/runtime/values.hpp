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
        ReturnSignal,
    };
}

struct RuntimeVal;

using Val = std::shared_ptr<RuntimeVal>;

struct RuntimeVal {
    ValueType::ValueType type;
    std::string value;
    std::unordered_map<std::string, Val> exports;

    RuntimeVal(ValueType::ValueType type) : type(type), value("") {}
    RuntimeVal(ValueType::ValueType type, const std::string& val) : type(type), value(val) {}
    virtual ~RuntimeVal() = default;

    virtual std::string toString() const {
        return "[object RuntimeVal]";
    }

    virtual double toNum() const {
        return 0;
    }

    virtual bool toBool() const {
        return true;
    }
};

using NativeFunction = std::function<Val(std::vector<Val>, Env*)>;

struct UndefinedVal : public RuntimeVal {
    UndefinedVal() : RuntimeVal(ValueType::Undefined, "undefined") {}
    std::string toString() const override { return "undefined"; }
    double toNum() const override { return 0; }
    bool toBool() const override { return false; }
};

struct NullVal : public RuntimeVal {
    NullVal() : RuntimeVal(ValueType::Null, "null") {}
    std::string toString() const override { return "null"; }
    double toNum() const override { return 0; }
    bool toBool() const override { return false; }
};

struct NumberVal : public RuntimeVal {
    double number;

    NumberVal(const std::string& val)
        : RuntimeVal(ValueType::Number, val), number(std::stod(val)) {}
    NumberVal(double val)
        : RuntimeVal(ValueType::Number), number(val) {}

    double getValue() { return number; }

    std::string toString() const override {
        return (std::floor(number) == number)
            ? std::to_string(static_cast<int>(number))
            : std::to_string(number);
    }

    double toNum() const override { return number; }
    bool toBool() const override { return number != 0; }
};

struct StringVal : public RuntimeVal {
    std::string string;
    StringVal(std::string val) : RuntimeVal(ValueType::String, val), string(val) {}
    std::string toString() const override { return string; }
    double toNum() const override { return stod(string); }
    bool toBool() const override { return !string.empty(); }
};

struct BooleanVal : public RuntimeVal {
    bool value;

    BooleanVal(bool val) 
        : RuntimeVal(ValueType::Boolean), value(val) {}

    bool getValue() const { return value; }

    std::string toString() const override { return value ? "true" : "false"; }
    double toNum() const override { return value ? 1 : 0; }
    bool toBool() const override { return value; }
};


struct ReturnSignal : public RuntimeVal {
    Val val;
    ReturnSignal(Val val)
    : RuntimeVal(ValueType::ReturnSignal), val(val) {}

};

struct ObjectVal : public RuntimeVal {
    std::unordered_map<std::string, Val> properties;
    ObjectVal(std::unordered_map<std::string, Val> properties = {})
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
    bool toBool() const override { return true; }
};

struct ArrayVal : public RuntimeVal {
    std::vector<Val> items;
    ArrayVal(std::vector<Val> items) : RuntimeVal(ValueType::Array), items(items) {}
    std::string toString() const override {
        std::string result = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            result += items[i]->toString();
            if (i < items.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
    bool toBool() const override { return true; }
};

struct NativeFnValue : public RuntimeVal {
    NativeFunction call;
    NativeFnValue(NativeFunction fn) : RuntimeVal(ValueType::NativeFn), call(fn) {}
    std::string toString() const override {
        return "[native function]";
    }
    bool toBool() const override { return true; }
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
    bool toBool() const override { return true; }
};

struct ProbeValue : public RuntimeVal {
    std::string name;
    Expr* extends;
    bool doesExtend = false;
    Env* declarationEnv;
    std::vector<Stmt*> body;
    ProbeValue (std::string name, Env* declarationEnv, std::vector<Stmt*> body) 
        : RuntimeVal(ValueType::Probe), name(name), declarationEnv(declarationEnv), body(body) {}
    ProbeValue (std::string name, Env* declarationEnv, std::vector<Stmt*> body, Expr* extends) 
        : RuntimeVal(ValueType::Probe), name(name), declarationEnv(declarationEnv), body(body), extends(extends), doesExtend(true) {}
    std::string toString() const override {
        return "[probe " + name + "]";
    }
    bool toBool() const override { return true; }
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
    bool toBool() const override { return true; }
};
