#pragma once

#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <memory>
#include <functional>
#include <algorithm>
#include <mutex>
#include <thread>

#include "instruction.hpp"
#include "values.hpp"
#include "utils.hpp"

namespace Probescript::VM
{

enum class SignalType
{
    Blank,

    Return,
};

struct Signal
{
    SignalType type;
    ValuePtr val;

    Signal(SignalType type = SignalType::Blank)
        : type(type) {}

    Signal(ValuePtr val)
        : type(SignalType::Return), val(val) {}
};

static std::shared_ptr<ObjectVal> s_Console = std::make_shared<ObjectVal>(std::unordered_map<std::string, ValuePtr>({
    {
        "println",
        std::make_shared<NativeFunctionVal>([](std::vector<ValuePtr> args) -> ValuePtr
        {
            size_t len = args.size();
            for (size_t i = 0; i < len; ++i)
            {
                auto arg = args[i];

                if (arg->type == ValueType::Number)
                {
                    std::cout << arg->toNum() << (args[i + 1] ? " " : "");
                }
                else
                {
                    std::cout << arg->toString() << (args[i + 1] ? " " : "");
                }
            }

            std::cout << '\n';
            return std::make_shared<NullVal>();
        })
    },
    {
        "print",
        std::make_shared<NativeFunctionVal>([](std::vector<ValuePtr> args) -> ValuePtr
        {
            size_t len = args.size();
            for (size_t i = 0; i < len; ++i)
            {
                auto arg = args[i];
                
                if (arg->type == ValueType::Number)
                {
                    std::cout << arg->toNum() << (args[i + 1] ? " " : "");
                }
                else
                {
                    std::cout << arg->toString() << (args[i + 1] ? " " : "");
                }
            }

            return std::make_shared<NullVal>();
        })
    },
    {
        "prompt",
        std::make_shared<NativeFunctionVal>([](std::vector<ValuePtr> args) -> ValuePtr
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
        : m_bytecode(bytecode), m_scope(scope), m_consts(consts) {}

    Signal run();
private:
    std::vector<std::shared_ptr<Instruction>> m_bytecode;
    std::vector<ValuePtr> m_stack;
    std::vector<ValuePtr> m_consts;
    ScopePtr m_scope;

    size_t ip = 0; // instruction pointer
    ValuePtr pop();
    void push(ValuePtr val);

    const ValuePtr s_null = std::make_shared<NullVal>();
};

} // namespace Probescript::VM