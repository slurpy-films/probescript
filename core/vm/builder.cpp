#include "builder.hpp"

using namespace Probescript::VM;

void ByteCodeBuilder::createNumber(double num)
{
    for (size_t i = 0; i < m_constants.size(); i++)
    {
        if (m_constants[i]->type == ValueType::Number && m_constants[i]->toNum() == num)
        {
            m_instructions.push_back(std::make_shared<Instruction>(Opcode::LOAD_CONST, i));
            return;
        }
    }

    m_constants.push_back(std::make_shared<NumberVal>(num));
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::LOAD_CONST, m_constants.size() - 1));
}

void ByteCodeBuilder::createString(const std::string& str)
{
    for (size_t i = 0; i < m_constants.size(); i++)
    {
        if (m_constants[i]->type == ValueType::String && m_constants[i]->toString() == str)
        {
            m_instructions.push_back(std::make_shared<Instruction>(Opcode::LOAD_CONST, i));
            return;
        }
    }

    m_constants.push_back(std::make_shared<StringVal>(str));
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::LOAD_CONST, m_constants.size() - 1));
}

void ByteCodeBuilder::createAdd()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::ADD));
}

void ByteCodeBuilder::createSub()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::SUB));
}

void ByteCodeBuilder::createMul()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::MUL));
}

void ByteCodeBuilder::createDiv()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::DIV));
}

void ByteCodeBuilder::createCall(size_t argc)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::CALL, static_cast<int>(argc)));
}

void ByteCodeBuilder::createPrint()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::PRINT));
}

void ByteCodeBuilder::createStore(const std::string& name)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::STORE, name));
}

void ByteCodeBuilder::createLoad(const std::string& name)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::LOAD, name));
}

void ByteCodeBuilder::createEquals()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::COMPARE, BoolOperator::EQUALS));
}

void ByteCodeBuilder::createJump(size_t line)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::JUMP, line, true));
}

void ByteCodeBuilder::createJumpIfFalse(size_t line)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::JUMP_IF_FALSE, line, true));
}

void ByteCodeBuilder::patchJumpIfFalse(size_t i, size_t line)
{
    m_instructions[i]->line = line;
}

void ByteCodeBuilder::createGreaterThan()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::COMPARE, BoolOperator::GREATER));
}

void ByteCodeBuilder::createLessThan()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::COMPARE, BoolOperator::LESS));
}

void ByteCodeBuilder::createAssign(const std::string& name)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::ASSIGN, name));
}

void ByteCodeBuilder::startIf()
{
    auto jumpIfFalse = std::make_shared<Instruction>(Opcode::JUMP_IF_FALSE, static_cast<size_t>(0), true);
    m_ifPatchStack.push_back(m_instructions.size());
    m_instructions.push_back(jumpIfFalse);
}

void ByteCodeBuilder::endIf()
{
    size_t jumpIndex = m_ifPatchStack.back();
    m_ifPatchStack.pop_back();

    size_t line = m_instructions.size();
    m_instructions[jumpIndex]->line = line;
}

void ByteCodeBuilder::startScope()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::START_SCOPE));
}

void ByteCodeBuilder::endScope()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::END_SCOPE));
}

void ByteCodeBuilder::startFunction()
{
    m_functionStack.push_back(std::make_shared<ByteCodeBuilder>(*this));
    m_instructions.clear();
}

void ByteCodeBuilder::endFunction(std::vector<std::string>& params)
{
    auto fnInstructions = m_instructions;

    m_instructions = m_functionStack.back()->m_instructions;
    m_functionStack.pop_back();

    m_instructions.push_back(std::make_shared<Instruction>(Opcode::MAKE_FUNCTION, fnInstructions, params));
}

void ByteCodeBuilder::startProbe()
{
    m_functionStack.push_back(std::make_shared<ByteCodeBuilder>(*this));
    m_instructions.clear();
}

void ByteCodeBuilder::endProbe(std::string probeName)
{
    auto prbInstructions = m_instructions;

    m_instructions = m_functionStack.back()->m_instructions;
    m_functionStack.pop_back();

    m_instructions.push_back(std::make_shared<Instruction>(probeName, Opcode::MAKE_PROBE, prbInstructions));
}

void ByteCodeBuilder::createNegate()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::NEGATE));
}

void ByteCodeBuilder::set(size_t i, std::shared_ptr<Instruction> instr)
{
    m_instructions[i] = instr;
}

void ByteCodeBuilder::createLoadGlobal(std::string globalName)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::LOAD_GLOBAL, globalName));
}

void ByteCodeBuilder::createReturn()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::RETURN));
}

void ByteCodeBuilder::createLoadConsole(std::string name)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::LOAD_CONSOLE, name));
}

void ByteCodeBuilder::createMemberAccess(std::string property)
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::ACCESS_PROPERTY, property));
}

size_t ByteCodeBuilder::getInstructionLength()
{
    return m_instructions.size();
}

std::vector<std::shared_ptr<Instruction>> ByteCodeBuilder::getInstructions()
{
    return m_instructions;
}

std::vector<ValuePtr> ByteCodeBuilder::getConstants()
{
    return m_constants;
}