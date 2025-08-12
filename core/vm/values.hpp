#pragma once

#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <future>

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
    Probe,
    Future,
    Class,
};

struct Value
{
    ValueType type;
    std::unordered_map<std::string, ValuePtr> properties;
    
    Value(ValueType type)
        : type(type) {}

    virtual std::string toString() const
    {
        return "null";
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
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(8) << number;
        std::string str = oss.str();

        str.erase(str.find_last_not_of('0') + 1, std::string::npos);

        if (!str.empty() && str.back() == '.')
            str.pop_back();

        return str;
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

    bool compare(const ValuePtr o) const override
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

struct NullVal : public Value
{
    bool isDefaultParam = false;

    NullVal(bool isDefaultParam = false)
        : Value(ValueType::Null), isDefaultParam(isDefaultParam) {}

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

struct FutureVal : public Value
{
    std::shared_future<ValuePtr> future;

    std::string toString() const override
    {
        std::string status = "pending";

        if (
            future.valid()
            && future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready
        )
        {
            status = "done";
        }

        return "[Future (" + status + ")]";
    }

    FutureVal(std::shared_future<ValuePtr> future)
        : Value(ValueType::Future), future(future) {}
};

struct FunctionContext
{
    std::vector<ValuePtr> constants;
    std::unordered_map<std::string, ValuePtr> globals;

    FunctionContext(std::vector<ValuePtr> constants, std::unordered_map<std::string, ValuePtr> globals)
        : constants(constants), globals(globals) {}
};

struct NativeFunctionVal : public Value
{
    using NativeFunction = std::function<ValuePtr(std::vector<ValuePtr>, std::shared_ptr<FunctionContext>)>;
    NativeFunction call;

    std::string toString() const override
    {
        return "[function]";
    }

    NativeFunctionVal(NativeFunction call)
        : Value(ValueType::NativeFunction), call(call) {}
};

struct ClassVal : public Value
{
    std::vector<std::shared_ptr<Instruction>> body;
    ScopePtr scope;

    std::string toString() const override
    {
        return "[class]";
    }

    ClassVal(std::vector<std::shared_ptr<Instruction>> body, ScopePtr scope)
        : Value(ValueType::Class), body(body), scope(scope) {}
};

struct NativeClassVal : public Value
{
    std::function<ValuePtr(std::vector<ValuePtr>)> call;

    std::string toString() const override
    {
        return "[class]";
    }

    NativeClassVal(std::function<ValuePtr(std::vector<ValuePtr>)> call)
        : Value(ValueType::NativeClass), call(call) {}
};

struct ArrayVal : public Value
{
    std::vector<ValuePtr> items;

    std::string toString() const override
    {
        std::ostringstream stream;
        stream << "[";

        size_t len = items.size();

        for (size_t i = 0; i < len; ++i)
        {
            stream << items[i]->toString() << (i + 1 < len ? ", " : "");
        }

        stream << "]";

        return stream.str();
    }

    ArrayVal(std::vector<ValuePtr> items = {})
        : Value(ValueType::Array), items(items) {}
};

struct FunctionValue : public Value
{
    std::vector<std::shared_ptr<Instruction>> body;
    ScopePtr scope;
    std::vector<std::string> parameters;
    bool async = false;

    std::string toString() const override
    {
        return "[function]";
    }

    FunctionValue(std::vector<std::shared_ptr<Instruction>> body, std::vector<std::string> parameters, ScopePtr scope)
        : Value(ValueType::Function), body(body), parameters(parameters), scope(scope) {}
};

struct ProbeValue : public Value
{
    std::vector<std::shared_ptr<Instruction>> body;
    ScopePtr scope;
    std::string name;

    ProbeValue(std::string name, std::vector<std::shared_ptr<Instruction>> body, ScopePtr scope)
        : Value(ValueType::Probe), name(name), body(body), scope(scope) {}
};

struct ObjectVal : public Value, std::enable_shared_from_this<ObjectVal>
{
private:
    std::string objectToString(ValuePtr prop, int depth = 0) const
    {
        if (depth > 3) return "[Object]"; // Prevent infinite recursion
        
        if (!prop) return "null";
        
        switch (prop->type)
        {
            case ValueType::Object:
            {
                auto obj = std::static_pointer_cast<ObjectVal>(prop);
                if (obj->properties.empty()) return "{}";
                
                std::ostringstream stream;
                std::ostringstream tabsStream;
                for (int _ = 0; _ < depth; ++_)
                {
                    tabsStream << "    ";
                }
                std::string tabs = tabsStream.str();

                stream << "\n" << tabs << "{";
                
                bool first = true;
                for (const auto& [key, value] : obj->properties)
                {
                    if (!first) stream << ", ";
                    stream << "\n" << tabs << "    ";
                    stream << '"' << key << '"' << ": " << objectToString(value, depth + 1);
                    first = false;
                }
                
                stream << "\n" <<  tabs << "}";
                return stream.str();
            }
            case ValueType::Array:
            {
                auto arr = std::static_pointer_cast<ArrayVal>(prop);
                if (arr->items.empty()) return "[]";
                
                std::ostringstream stream;
                stream << "[ ";
                
                for (size_t i = 0; i < arr->items.size(); ++i)
                {
                    if (i > 0) stream << ", ";
                    stream << objectToString(arr->items[i], depth + 1);
                }
                
                stream << " ]";
                return stream.str();
            }
            case ValueType::String:
                return "\"" + prop->toString() + "\"";
            default:
                return prop->toString();
        }
    }

public:
    bool hasProperty(std::string prop)
    {
        return properties.find(prop) != properties.end();
    }

    std::string toString() const override
    {
        return objectToString(std::const_pointer_cast<ObjectVal>(shared_from_this()));
    }

    ObjectVal(std::unordered_map<std::string, ValuePtr> properties = {})
        : Value(ValueType::Object)
    {
        this->properties = properties;
    }
};

} // namespace Probescript::VM