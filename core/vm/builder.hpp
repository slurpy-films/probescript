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

    void createBoolLiteral(bool value);

    void createStore(const std::string& name);
    void createLoad(const std::string& name);
    void createAssign(const std::string& name);
    
    void createAdd();
    void createSub();
    void createMul();
    void createDiv();
    
    void createLoadGlobal(std::string globalName);
    void createLoadConsole(std::string name = "");

    void createMemberAccess(std::string prop = "");

    void createReturn();

    void createEquals();
    void createGreaterThan();
    void createLessThan();
    void createOr();
    void createAnd();

    void createJump(size_t line);
    
    void createJumpIfFalse(size_t line);
    void patchJumpIfFalse(size_t i, size_t line);

    void startFunction();
    void endFunction(std::vector<std::string>& params);
    
    void startProbe();
    void endProbe(std::string probeName);

    void createNegate();

    void startIf();
    void endIf();

    void startScope();
    void endScope();

    void set(size_t index, std::shared_ptr<Instruction> instr);

    size_t getInstructionLength();

    std::vector<std::shared_ptr<Instruction>> getInstructions();
    std::vector<ValuePtr> getConstants();
private:
    std::vector<std::shared_ptr<Instruction>> m_instructions;
    std::vector<ValuePtr> m_constants;

    std::vector<std::shared_ptr<ByteCodeBuilder>> m_functionStack;
    std::vector<std::shared_ptr<ByteCodeBuilder>> m_ifStack;

    // Since the JUMP_IF_FALSE instruction increments the instruction pointer instead of setting it, we need to keep track of the line number
    std::vector<size_t> m_ifPatchStack;

    template <typename T, typename... Args>
    std::shared_ptr<T> mk(Args&&... args)
    {
        return std::make_shared<T>(std::forward(args)...);
    }
};

} // namespace Probescript::VM