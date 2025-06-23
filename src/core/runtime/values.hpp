#pragma once
#include <string>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <functional>
#include "frontend/ast.hpp"
#include "utils.hpp"

class Env;
using EnvPtr = std::shared_ptr<Env>;

enum class ValueType {
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
    NativeClass,
    BreakSignal,
    ContinueSignal,
};

struct RuntimeVal;
struct NumberVal;
using Val = std::shared_ptr<RuntimeVal>;

struct RuntimeVal {
    ValueType type;
    std::string value;
    std::unordered_map<std::string, Val> exports;
    std::unordered_map<std::string, Val> properties;
    RuntimeVal(ValueType type, std::unordered_map<std::string, Val> properties) : type(type), properties(properties) {}
    RuntimeVal(ValueType type) : type(type), value("") {}
    RuntimeVal(ValueType type, const std::string& val) : type(type), value(val) {}
    virtual ~RuntimeVal() = default;

    virtual std::string toString() const {
        return "[object RuntimeVal]";
    }

    virtual std::string toJSON() const {
        return "null";
    }

    virtual double toNum() const {
        return 0;
    }

    virtual bool toBool() const {
        return true;
    }

    virtual std::string toConsole() const {
        return toString();
    }

    virtual bool compare(const RuntimeVal& other) const
    {
        return false;
    }

    bool operator == (const RuntimeVal& other) const
    {
        return this->compare(other);
    }

    virtual Val add(Val o) const;
    virtual Val sub(Val o) const;
    virtual Val mul(Val o) const;
    virtual Val div(Val o) const;
    virtual Val mod(Val o) const;
};

using NativeFunction = std::function<Val(std::vector<Val>, EnvPtr)>;

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

    std::string toConsole() const override {
        return ConsoleColors::YELLOW + (std::floor(number) == number
            ? std::to_string(static_cast<int>(number))
            : std::to_string(number)) + ConsoleColors::RESET;
    }

    std::string toJSON() const override {
        return std::to_string(number);
    }

    double toNum() const override { return number; }
    bool toBool() const override { return number != 0; }

    Val add(Val o) const override {
        return std::make_shared<NumberVal>(number + o->toNum());
    }

    Val sub(Val o) const override {
        return std::make_shared<NumberVal>(number - o->toNum());
    }

    Val mul(Val o) const override {
        return std::make_shared<NumberVal>(number * o->toNum());
    }

    Val div(Val o) const override {
        return std::make_shared<NumberVal>(number / o->toNum());
    }

    Val mod(Val o) const override {
        return std::make_shared<NumberVal>(fmod(number, o->toNum()));
    }

    bool compare(const RuntimeVal& other) const override
    {
        if (other.type != ValueType::Number)
            return false;
    
        const NumberVal& num = static_cast<const NumberVal&>(other);
        return num.number == number;
    }
};

struct UndefinedVal : public RuntimeVal {
    UndefinedVal() : RuntimeVal(ValueType::Undefined, "undefined") {}
    std::string toString() const override { return "undefined"; }
    double toNum() const override { return 0; }
    bool toBool() const override { return false; }
    std::string toJSON() const override {
        return "null";
    }
    std::string toConsole() const override {
        return ConsoleColors::GRAY + "undefined" + ConsoleColors::RESET;
    }

    Val add(Val o) const override {
        return std::make_shared<NumberVal>(0 + o->toNum());
    }

    Val sub(Val o) const override {
        return std::make_shared<NumberVal>(0 - o->toNum());
    }

    bool compare(const RuntimeVal& other) const override
    {
        return other.type == ValueType::Undefined;
    }
};

struct NullVal : public RuntimeVal {
    NullVal() : RuntimeVal(ValueType::Null, "null") {}
    std::string toString() const override { return "null"; }
    double toNum() const override { return 0; }
    bool toBool() const override { return false; }
    std::string toJSON() const override {
        return "null";
    }
    std::string toConsole() const override {
        return ConsoleColors::GRAY + "null" + ConsoleColors::RESET;
    }

    Val add(Val o) const override {
        return std::make_shared<NumberVal>(0 + o->toNum());
    }

    Val sub(Val o) const override {
        return std::make_shared<NumberVal>(0 - o->toNum());
    }

    bool compare(const RuntimeVal& other) const override
    {
        return other.type == ValueType::Null;
    }
};

struct NativeFnValue : public RuntimeVal {
    NativeFunction call;
    NativeFnValue(NativeFunction fn) : RuntimeVal(ValueType::NativeFn), call(fn) {}
    std::string toString() const override {
        return "[native function]";
    }
    bool toBool() const override { return true; }

    bool compare(const RuntimeVal& other) const override
    {
        return false;
    }
};

struct BooleanVal : public RuntimeVal {
    bool value;

    BooleanVal(bool val) 
        : RuntimeVal(ValueType::Boolean), value(val) {}

    bool getValue() const { return value; }

    std::string toString() const override { return value ? "true" : "false"; }
    double toNum() const override { return value ? 1 : 0; }
    bool toBool() const override { return value; }
    std::string toJSON() const override {
        return value ? "true" : "false";
    }

    std::string toConsole() const override {
        return ConsoleColors::YELLOW + (value ? "true" : "false") + ConsoleColors::RESET;
    }

    Val add(Val o) const override {
        return std::make_shared<NumberVal>((value ? 1 : 0) + o->toNum());
    }

    Val sub(Val o) const override {
        return std::make_shared<NumberVal>((value ? 1 : 0) - o->toNum());
    }

    Val mul(Val o) const override {
        return std::make_shared<NumberVal>((value ? 1 : 0) * o->toNum());
    }

    Val div(Val o) const override {
        return std::make_shared<NumberVal>((value ? 1 : 0) / o->toNum());
    }

    Val mod(Val o) const override {
        return std::make_shared<NumberVal>(fmod((value ? 1 : 0), o->toNum()));
    }

    bool compare(const RuntimeVal& other) const override
    {
        if (other.type != ValueType::Boolean)
            return false;
    
        const BooleanVal& boolean = static_cast<const BooleanVal&>(other);
        return boolean.value == value;
    }
};


struct ReturnSignal : public RuntimeVal {
    Val val;
    ReturnSignal(Val val)
    : RuntimeVal(ValueType::ReturnSignal), val(val) {}
    std::string toJSON() const override {
        return "null";
    }
};

struct BreakSignal : public RuntimeVal {
    std::string toJSON() const override {
        return "null";
    }
    BreakSignal() : RuntimeVal(ValueType::BreakSignal) {}
};

struct ContinueSignal : public RuntimeVal {
    std::string toJSON() const override {
        return "null";
    }
    ContinueSignal() : RuntimeVal(ValueType::ContinueSignal) {}
};

struct ArrayVal : public RuntimeVal {
    std::vector<Val> items;
    ArrayVal(std::vector<Val> items = {});
    std::string toString() const override {
        std::string result = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            result += items[i]->toString();
            if (i < items.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
    std::string toJSON() const override {
        std::string result = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            result += items[i]->toJSON();
            if (i < items.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
    std::string toConsole() const override {
        std::string result = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            result += items[i]->toConsole();
            if (i < items.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
    bool toBool() const override { return true; }

    Val add(Val o) const override {
        std::vector<Val> citems(items);
        citems.push_back(o);
        return std::make_shared<ArrayVal>(citems);
    }

    bool compare(const RuntimeVal& other) const override
    {
        if (other.type != ValueType::Array)
            return false;
    
        const ArrayVal& arr = static_cast<const ArrayVal&>(other);
        if (arr.items.size() != items.size())
            return false;

        for (size_t i = 0; i < items.size(); i++)
        {
            if (!(*arr.items[i] == *items[i]))
                return false;
        }

        return true;
    }
};



struct FunctionValue : public RuntimeVal {
    std::string name;
    std::vector<VarDeclarationType*> params;
    std::vector<VarDeclarationType*> templateparams;
    EnvPtr declarationEnv;
    std::vector<Stmt*> body;
    FunctionValue (std::string name, std::vector<VarDeclarationType*> params, EnvPtr declarationEnv, std::vector<Stmt*> body) 
        : RuntimeVal(ValueType::Function), name(name), params(params), declarationEnv(declarationEnv), body(body) {}
    std::string toString() const override {
        return "[function " + name + "]";
    }
    std::string toJSON() const override {
        return "null";
    }
    std::string toConsole() const override {
        return ConsoleColors::CYAN + "[function " + name + "]" + ConsoleColors::RESET;
    }
    bool toBool() const override { return true; }
};

struct ProbeValue : public RuntimeVal {
    std::string name;
    Expr* extends;
    bool doesExtend = false;
    EnvPtr declarationEnv;
    std::vector<Stmt*> body;
    ProbeValue (std::string name, EnvPtr declarationEnv, std::vector<Stmt*> body) 
        : RuntimeVal(ValueType::Probe), name(name), declarationEnv(declarationEnv), body(body) {}
    ProbeValue (std::string name, EnvPtr declarationEnv, std::vector<Stmt*> body, Expr* extends) 
        : RuntimeVal(ValueType::Probe), name(name), declarationEnv(declarationEnv), body(body), extends(extends), doesExtend(true) {}
    std::string toString() const override {
        return "[probe " + name + "]";
    }
    std::string toJSON() const override {
        return "null";
    }
    bool toBool() const override { return true; }
};

struct ClassVal : public RuntimeVal {
    std::string name;
    EnvPtr parentEnv;
    std::vector<Stmt*> body;
    Expr* extends;
    bool doesExtend = false;
    ClassVal(std::string name, EnvPtr declarationEnv, std::vector<Stmt*> body) 
    : RuntimeVal(ValueType::Class), name(name), parentEnv(declarationEnv), body(body) {}
    ClassVal(std::string name, EnvPtr declarationEnv, std::vector<Stmt*> body, Expr* extends) 
    : RuntimeVal(ValueType::Class), name(name), parentEnv(declarationEnv), body(body), extends(extends), doesExtend(true) {}
    std::string toString() const override {
        return "[class " + name + "]";
    }
    bool toBool() const override { return true; }
};

struct NativeClassVal : public RuntimeVal {
    NativeFunction constructor;
    NativeClassVal(NativeFunction constructor)
        : RuntimeVal(ValueType::NativeClass), constructor(constructor) {}

    std::string toString() const override {
        return "[native class]";
    }

    std::string toJSON() const override {
        return "[native class]";
    }
};

struct StringVal : public RuntimeVal {
    std::string string;
    StringVal(std::string val) : RuntimeVal(ValueType::String), string(val) {
        properties = 
        {
            {
                "length",
                std::make_shared<NativeFnValue>([this](std::vector<Val> _args, EnvPtr _env) -> Val {
                    return std::make_shared<NumberVal>(this->string.length());
                })
            },
            {
                "split",
                std::make_shared<NativeFnValue>([this](std::vector<Val> args, EnvPtr _) -> Val {
                    if (args.empty() || args[0]->type != ValueType::String) return std::make_shared<UndefinedVal>();
                    std::string str = this->string;
                    std::string deli = std::static_pointer_cast<StringVal>(args[0])->string;

                    std::vector<Val> result;
                    if (deli == "") {
                        std::vector<std::string> chars = splitToChars(str);

                        for (auto& c : chars) {
                            result.push_back(std::make_shared<StringVal>(c));
                        }

                        return std::make_shared<ArrayVal>(result);
                    }
                    size_t pos = 0;
                    std::string token;
                    while ((pos = str.find(deli)) != std::string::npos) {
                        token = str.substr(0, pos);
                        result.push_back(std::make_shared<StringVal>(token));
                        str.erase(0, pos + deli.length());
                    }
                    return std::make_shared<ArrayVal>(result);
                })
            }
        };
    }
    std::string toString() const override { return string; }
    std::string toJSON() const override { return "\"" + string + "\""; }
    double toNum() const override
    {
        double val;
        try
        {
            val = stod(string);
        } catch (...)
        {
            val = 0;
        }
        return val;
    }
    bool toBool() const override { return !string.empty(); }
    std::string toConsole() const override {
        return ConsoleColors::GREEN + "\"" + string + "\"" + ConsoleColors::RESET;
    }

    Val add(Val o) const override {
        return std::make_shared<StringVal>(string + o->toString());
    }

    Val mul(Val o) {
        std::string r;
        for (size_t i = 0; i < o->toNum() && i < 10000; i++) {
            r += string;
        }

        return std::make_shared<StringVal>(r);
    }

    bool compare(const RuntimeVal& other) const override
    {
        if (other.type != ValueType::String)
            return false;
    
        const StringVal& str = static_cast<const StringVal&>(other);
        return str.string == string;
    }
};


struct ObjectVal : public RuntimeVal {
    ObjectVal(std::unordered_map<std::string, Val> properties = {})
        : RuntimeVal(ValueType::Object, properties) {
            properties["hasProperty"] = std::make_shared<NativeFnValue>([this](std::vector<Val> args, EnvPtr env) -> Val {
                if (args.empty() || args[0]->type != ValueType::String) return std::make_shared<BooleanVal>(false);

                return std::make_shared<BooleanVal>(this->hasProperty(std::static_pointer_cast<StringVal>(args[0])->string));
            });
        }

    bool hasProperty(const std::string& prop) {
        return (properties.find(prop) != properties.end());
    }

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
    std::string toJSON() const override {
        std::string result = "{ ";
        bool first = true;
        for (const auto& [key, val] : properties) {
            if (!first) result += ", ";
            result += "\"" + key + "\"" + ": " + val->toJSON();
            first = false;
        }
        result += " }";
        return result;
    }
    std::string toConsole() const override {
        std::string result = "{ ";
        bool first = true;
        for (const auto& [key, val] : properties) {
            if (!first) result += ", ";
            result += "\"" + key + "\"" + ": " + val->toConsole();
            first = false;
        }
        result += " }";
        return result;
    }
    bool toBool() const override { return true; }

    bool compare(const RuntimeVal& other) const override
    {
       return false;
    }
};