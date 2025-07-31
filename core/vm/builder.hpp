#pragma once
#include <string>
#include <vector>

#include "instruction.hpp"
#include "values.hpp"

namespace Probescript::VM
{

class ByteCodeBuilder
{
public:
    void createNumber(double num);
    void createString(const std::string& str);
    void createNull();
    
    void createCall(size_t argc);
    void createNew(size_t argc);

    void createLoadStdlib(const std::string& name);
    
    void createPrint();

    void createBoolLiteral(bool value);

    void createStore(const std::string& name);
    void createLoad(const std::string& name);
    void createAssign(const std::string& name);
    
    void createAdd();
    void createSub();
    void createMul();
    void createDiv();

    void createArray();
    void pushToArray();

    void createThrow();

    void createPop();
    
    void createLoadGlobal(std::string globalName);
    void createLoadConsole(std::string name = "");

    void createObject();
    void createMemberAccess(std::string prop = "");
    void createMemberAssign(std::string prop = "");

    void createReturn();

    void createEquals();
    void createNotEquals();
    void createGreaterThan();
    void createLessThan();
    void createGreaterThanOrEqualTo();
    void createLessThanOrEqualTo();
    void createOr();
    void createAnd();

    void createJump(size_t line);
    
    void createJumpIfFalse(size_t line);
    void patchJumpIfFalse(size_t i, size_t line);

    void startFunction();
    void endFunction(std::vector<std::string>& params, const std::string& name);

    void startClass();
    void endClass(bool extends);

    void createExport(const std::string& name);
    
    void startProbe();
    void endProbe(const std::string& probeName);

    void createNegate();

    void startIf();
    void endIf();

    void startScope();
    void endScope();

    void startModule();
    void endModule();

    void set(size_t index, std::shared_ptr<Instruction> instr);

    size_t getVarCounter(); // Get a unique number
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

    size_t m_couter = 0;

    template <typename T, typename... Args>
    std::shared_ptr<T> mk(Args&&... args)
    {
        return std::make_shared<T>(std::forward(args)...);
    }
};

} // namespace Probescript::VM
