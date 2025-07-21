#include "builder.hpp"

using namespace Probescript::VM;

void ByteCodeBuilder::createNumber(double num)
{
    for (size_t i = 0; i < m_constants.size(); i++)
    {
        if (m_constants[i]->type == ValueType::Number && m_constants[i]->toNum() == num)
        {
            m_instructions.push_back(std::make_shared<LOAD_CONST>((size_t)i));
            return;
        }
    }

    m_constants.push_back(std::make_shared<NumberVal>(num));
    m_instructions.push_back(std::make_shared<LOAD_CONST>(m_constants.size() - 1));
}

void ByteCodeBuilder::createString(const std::string& str)
{
    for (size_t i = 0; i < m_constants.size(); i++)
    {
        if (m_constants[i]->type == ValueType::String && m_constants[i]->toString() == str)
        {
            m_instructions.push_back(std::make_shared<LOAD_CONST>((size_t)i));
            return;
        }
    }

    m_constants.push_back(std::make_shared<StringVal>(str));
    m_instructions.push_back(std::make_shared<LOAD_CONST>((size_t)(m_constants.size() - 1)));
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
    m_instructions.push_back(std::make_shared<CALL>(argc));
}

void ByteCodeBuilder::createPrint()
{
    m_instructions.push_back(std::make_shared<Instruction>(Opcode::PRINT));
}

void ByteCodeBuilder::createStore(const std::string& name)
{
    m_instructions.push_back(std::make_shared<STORE>(name));
}

void ByteCodeBuilder::createLoad(const std::string& name)
{
    m_instructions.push_back(std::make_shared<LOAD>(name));
}

void ByteCodeBuilder::startFunction()
{
    m_functionStack.push_back(std::make_shared<ByteCodeBuilder>(*this));
    m_instructions.clear();
    m_isBuildingFunction = true;
}

void ByteCodeBuilder::endFunction()
{
    auto fnInstructions = m_instructions;

    m_instructions = m_functionStack.back()->m_instructions;
    m_functionStack.pop_back();
    m_isBuildingFunction = !m_functionStack.empty();

    m_instructions.push_back(std::make_shared<MAKE_FUNCTION>(fnInstructions));
}

std::vector<std::shared_ptr<Instruction>> ByteCodeBuilder::getInstructions()
{
    return m_instructions;
}

std::vector<ValuePtr> ByteCodeBuilder::getConstants()
{
    return m_constants;
}