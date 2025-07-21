#pragma once

#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <memory>
#include <functional>

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

    ValuePtr declareVar(std::string name, ValuePtr value);
    ValuePtr lookupVar(std::string name);
private:
    ScopePtr m_parent;
    std::unordered_map<std::string, ValuePtr> m_variables;
    bool m_ready = false;
    
    void init();
    ScopePtr resolve(std::string varName);
};

enum class ValueType
{
    String,
    Number,
    Function,
    NativeFunction,
    Null,
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
};

struct NullVal : public Value
{
    NullVal()
        : Value(ValueType::Null) {}

    std::string toString() const override
    {
        return "null";
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
};

struct NativeFunctionVal : public Value
{
    std::function<ValuePtr(std::vector<ValuePtr>)> call;

    NativeFunctionVal(std::function<ValuePtr(std::vector<ValuePtr>)> call)
        : Value(ValueType::NativeFunction), call(call) {}
};

struct FunctionValue : public Value
{
    std::vector<std::shared_ptr<Instruction>> body;
    ScopePtr scope;

    FunctionValue(std::vector<std::shared_ptr<Instruction>> body, ScopePtr scope)
        : Value(ValueType::Function), body(body), scope(scope) {}
};

} // namespace Probescript::VM