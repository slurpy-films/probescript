#pragma once
#include <string>
#include <vector>

#include "instruction.hpp"
#include "vm.hpp"

namespace Probescript::VM
{

class ByteCodeBuilder
{
public:
    void createNumber(double num);
    void createString(const std::string& str);
    void createCall(size_t argc);
    void createPrint();
    void createStore(const std::string& name);
    void createLoad(const std::string& name);
    
    void createAdd();
    void createSub();
    void createMul();
    void createDiv();

    void startFunction();
    void endFunction();

    std::vector<std::shared_ptr<Instruction>> getInstructions();
    std::vector<ValuePtr> getConstants();
private:
    std::vector<std::shared_ptr<Instruction>> m_instructions;
    std::vector<ValuePtr> m_constants;
    std::vector<std::shared_ptr<ByteCodeBuilder>> m_functionStack;
    bool m_isBuildingFunction = false;

    template <typename T, typename... Args>
    std::shared_ptr<T> mk(Args&&... args)
    {
        return std::make_shared<T>(std::forward(args)...);
    }
};

} // namespace Probescript::VM