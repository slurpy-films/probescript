#pragma once

#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <memory>
#include <functional>

#include "instruction.hpp"
#include "values.hpp"

namespace Probescript::VM
{

class Machine
{
public:
    Machine(const std::vector<std::shared_ptr<Instruction>>& bytecode, std::vector<ValuePtr> consts, ScopePtr scope)
        : m_bytecode(bytecode), m_scope(scope), m_consts(consts) {}

    void run();
private:
    std::vector<std::shared_ptr<Instruction>> m_bytecode;
    std::vector<ValuePtr> m_stack;
    std::vector<ValuePtr> m_consts;
    ScopePtr m_scope;

    size_t ip = 0; // instruction pointer
    ValuePtr pop();
    void push(ValuePtr val);

    void runCall(std::shared_ptr<CALL> call);
    void runLoadConst(std::shared_ptr<LOAD_CONST> loadConst);
    void runLoad(std::shared_ptr<LOAD> load);
    void runStore(std::shared_ptr<STORE> store);
    void runPrint();
};

} // namespace Probescript::VM