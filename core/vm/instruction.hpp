#pragma once
#include <string>
#include <vector>
#include <variant>
#include <memory>

namespace Probescript::VM
{

enum class Opcode
{
    LOAD_CONST,
    PRINT,
    ADD,
    SUB,
    MUL,
    DIV,
    HALT,
    CALL,
    STORE,
    LOAD,
    MAKE_FUNCTION,
    COMPARE,
    JUMP_IF_FALSE,
    START_SCOPE,
    END_SCOPE,
    JUMP,
    ASSIGN,
};

inline std::string OpCodeToString(Opcode code)
{
    switch (code)
    {
        case Opcode::LOAD_CONST:
            return "LOAD_CONST";
        case Opcode::PRINT:
            return "PRINT";
        case Opcode::ADD:
            return "ADD";
        case Opcode::SUB:
            return "SUB";
        case Opcode::MUL:
            return "MUL";
        case Opcode::DIV:
            return "DIV";
        case Opcode::HALT:
            return "HALT";
        case Opcode::CALL:
            return "CALL";
        case Opcode::STORE:
            return "STORE";
        case Opcode::LOAD:
            return "LOAD";
        case Opcode::MAKE_FUNCTION:
            return "MAKE_FUNCTION";
        case Opcode::COMPARE:
            return "COMPARE";
        case Opcode::JUMP_IF_FALSE:
            return "JUMP_IF_FALSE";
        case Opcode::START_SCOPE:
            return "START_SCOPE";
        case Opcode::END_SCOPE:
            return "END_SCOPE";
        case Opcode::JUMP:
            return "JUMP";
        case Opcode::ASSIGN:
            return "ASSIGN";
        default:
            return "UNKNOWN";
    }
}

enum class BoolOperator
{
    EQUALS,
    GREATER,
    LESS,
};

struct Instruction
{
    Opcode op;
    size_t index;
    int argc;
    std::string name;
    std::vector<std::shared_ptr<Instruction>> body;
    BoolOperator boolop;
    size_t line;

    explicit Instruction(Opcode op) : op(op), index(0), argc(0), boolop(BoolOperator::EQUALS), line(0) {}
    
    Instruction(Opcode op, size_t index) : op(op), index(index), argc(0), boolop(BoolOperator::EQUALS), line(0) {}
    
    Instruction(Opcode op, int argc) : op(op), index(0), argc(argc), boolop(BoolOperator::EQUALS), line(0) {}
    
    Instruction(Opcode op, const std::string& name) : op(op), index(0), argc(0), name(name), boolop(BoolOperator::EQUALS), line(0) {}
    
    Instruction(Opcode op, const std::vector<std::shared_ptr<Instruction>>& body) : op(op), index(0), argc(0), body(body), boolop(BoolOperator::EQUALS), line(0) {}
    
    Instruction(Opcode op, BoolOperator boolop) : op(op), index(0), argc(0), boolop(boolop), line(0) {}
    
    Instruction(Opcode op, size_t line, bool) : op(op), index(0), argc(0), boolop(BoolOperator::EQUALS), line(line) {}
};

} // namespace Probescript::VM