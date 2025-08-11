#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <future>

#include <unordered_map>

#include "instruction.hpp"
#include "values.hpp"

extern std::unordered_map<std::string, Probescript::VM::ValuePtr> g_valueGlobals;

namespace Probescript::VM
{

ValuePtr call(ValuePtr fn, std::vector<ValuePtr> args, std::shared_ptr<FunctionContext> context);

enum class SignalType
{
    Blank,

    Return,
};

struct Signal
{
    SignalType type;
    ValuePtr val;
    std::unordered_map<std::string, ValuePtr> exports;

    Signal(SignalType type = SignalType::Blank)
        : type(type) {}

    Signal(ValuePtr val)
        : type(SignalType::Return), val(val) {}
};

static std::shared_ptr<ObjectVal> s_Console = std::make_shared<ObjectVal>(std::unordered_map<std::string, ValuePtr>({
    {
        "println",
        std::make_shared<NativeFunctionVal>([](std::vector<ValuePtr> args, std::shared_ptr<FunctionContext> ctx) -> ValuePtr
        {
            size_t len = args.size();
            for (size_t i = 0; i < len; ++i)
            {
                std::cout << args[i]->toString();
                if (i + 1 < args.size()) std::cout << " ";
            }
            std::cout << '\n';
            return std::make_shared<NullVal>();
        })
    },
    {
        "print",
        std::make_shared<NativeFunctionVal>([](std::vector<ValuePtr> args, std::shared_ptr<FunctionContext> ctx) -> ValuePtr
        {
            size_t len = args.size();
            for (size_t i = 0; i < len; ++i)
            {
                std::cout << args[i]->toString();
                if (i + 1 < args.size()) std::cout << " ";
            }

            return std::make_shared<NullVal>();
        })
    },
    {
        "prompt",
        std::make_shared<NativeFunctionVal>([](std::vector<ValuePtr> args, std::shared_ptr<FunctionContext> ctx) -> ValuePtr
        {
            for (auto& arg : args)
            {
                if (arg->type == ValueType::Number)
                {
                    std::cout << arg->toNum();
                }
                else
                {
                    std::cout << arg->toString();
                }
            }

            std::string input;
            std::getline(std::cin, input);
            return std::make_shared<StringVal>(input);
        })
    }
}));

class Machine
{
public:
    Machine(const std::vector<std::shared_ptr<Instruction>>& bytecode, std::vector<ValuePtr> consts, ScopePtr scope)
        : m_bytecode(bytecode), m_scope(scope), m_consts(consts), m_globals(g_valueGlobals) {}

    Machine()
        : m_globals(g_valueGlobals) {}

    void load(const std::vector<std::shared_ptr<Instruction>>& bytecode, std::vector<ValuePtr> consts);

    Signal run();

    void registerGlobal(const std::string& name, ValuePtr value);

    ValuePtr lookup(std::string name);
    std::unordered_map<std::string, ValuePtr> getGlobals();
private:
    std::vector<std::shared_ptr<Instruction>> m_bytecode;
    std::vector<ValuePtr> m_stack;
    std::vector<ValuePtr> m_consts;
    ScopePtr m_scope;
    std::unordered_map<std::string, ValuePtr> m_exports;
    std::unordered_map<std::string, ValuePtr> m_globals;

    // This variable will be incremented by one every time START_SCOPE is called to ensure proper cleanup
    int m_scopeCount = 0;

    size_t ip = 0; // instruction pointer
    ValuePtr pop();
    void push(ValuePtr val);

    Signal runInstruction(std::shared_ptr<Instruction> instr);

    const ValuePtr s_null = std::make_shared<NullVal>();
};

} // namespace Probescript::VM
