#pragma once

#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>

#include "instruction.hpp"

namespace Probescript::VM
{

struct Value;
using ValuePtr = std::shared_ptr<Value>;

class Scope;
using ScopePtr = std::shared_ptr<Scope>;

class Scope : public std::enable_shared_from_this<Scope>
{
public:
    Scope(ScopePtr parent = nullptr)
        : m_parent(parent) {}

    ValuePtr declare(const std::string& name, ValuePtr value);
    ValuePtr assign(const std::string& name, ValuePtr value);
    ValuePtr lookupVar(const std::string& name);

    ScopePtr getParent();
private:
    ScopePtr m_parent;
    std::unordered_map<std::string, ValuePtr> m_variables;
    bool m_ready = false;
    
    void init();
    ScopePtr resolve(const std::string& varName);
};

enum class ValueType
{
    String,
    Number,
    Function,
    NativeFunction,
    Null,
    Boolean,
    Object,
    NativeClass,
    Array,
};

struct Value
{
    ValueType type;
    Value(ValueType type)
        : type(type) {}

    virtual std::string toString() const
    {
        return "";
    }

    virtual double toNum() const
    {
        return 0;
    }
    
    virtual bool toBool() const
    {
        return false;
    }

    virtual ValuePtr add(const ValuePtr o) const;
    virtual ValuePtr sub(const ValuePtr o) const;
    virtual bool compare(const ValuePtr o) const;
};

struct NumberVal : public Value
{
    double number;

    NumberVal(double number)
        : Value(ValueType::Number), number(number) {}

    double toNum() const override
    {
        return number;
    }

    std::string toString() const override
    {
        return std::to_string(number);
    }

    bool toBool() const override
    {
        return number != 0;
    }

    ValuePtr add(const ValuePtr o) const override
    {
        return std::make_shared<NumberVal>(number + o->toNum());
    }

    ValuePtr sub(const ValuePtr o) const override
    {
        return std::make_shared<NumberVal>(number - o->toNum());
    }

    bool compare(const ValuePtr o) const override
    {
        return o->type == ValueType::Number && o->toNum() == number;
    }
};

struct BooleanVal : public Value
{
    bool boolean;

    std::string toString() const override
    {
        return boolean ? "true" : "false";
    }

    bool compare(const ValuePtr o)
    {
        return o->type == ValueType::Boolean && o->toBool() == boolean;
    }

    bool toBool() const override
    {
        return boolean;
    }

    BooleanVal(bool val)
        : Value(ValueType::Boolean), boolean(val) {}
};

struct ObjectVal : public Value
{
    std::unordered_map<std::string, ValuePtr> properties;

    ObjectVal(std::unordered_map<std::string, ValuePtr> properties = {})
        : Value(ValueType::Object), properties(properties) {}
};

struct NullVal : public Value
{
    NullVal()
        : Value(ValueType::Null) {}

    std::string toString() const override
    {
        return "null";
    }
    
    ValuePtr add(const ValuePtr o) const override
    {
        return std::make_shared<NullVal>();
    }

    bool compare(const ValuePtr o) const override
    {
        return o->type == ValueType::Null;
    }
};

struct StringVal : public Value
{
    std::string string;

    StringVal(std::string string)
        : Value(ValueType::String), string(string) {}

    std::string toString() const override
    {
        return string;    
    }

    double toNum() const override
    {
        try
        {
            return std::stod(string);
        }
        catch(...)
        {
            return 0;
        }
    }

    bool toBool() const override
    {
        return !string.empty();
    }

    ValuePtr add(const ValuePtr o) const override
    {
        return std::make_shared<StringVal>(string + o->toString());
    }

    bool compare(const ValuePtr o) const override
    {
        return o->type == ValueType::String && o->toString() == string;
    }
};

struct NativeFunctionVal : public Value
{
    std::function<ValuePtr(std::vector<ValuePtr>)> call;

    NativeFunctionVal(std::function<ValuePtr(std::vector<ValuePtr>)> call)
        : Value(ValueType::NativeFunction), call(call) {}
};

struct NativeClassVal : public Value
{
    std::function<ValuePtr(std::vector<ValuePtr>)> call;

    NativeClassVal(std::function<ValuePtr(std::vector<ValuePtr>)> call)
        : Value(ValueType::NativeClass), call(call) {}
};

struct ArrayVal : public Value
{
    std::vector<ValuePtr> items;

    ArrayVal(std::vector<ValuePtr> items = {})
        : Value(ValueType::Array), items(items) {}
};

struct FunctionValue : public Value
{
    std::vector<std::shared_ptr<Instruction>> body;
    ScopePtr scope;

    FunctionValue(std::vector<std::shared_ptr<Instruction>> body, ScopePtr scope)
        : Value(ValueType::Function), body(body), scope(scope) {}
};

} // namespace Probescript::VM