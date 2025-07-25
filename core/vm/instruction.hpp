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
    MAKE_PROBE,
    COMPARE,
    JUMP_IF_FALSE,
    START_SCOPE,
    END_SCOPE,
    JUMP,
    ASSIGN,
    ACCESS_PROPERTY,
    LOAD_GLOBAL,
    LOAD_CONSOLE, // Special instruction for loading a console property like println
    RETURN,
    NEGATE,
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
        case Opcode::MAKE_PROBE:
            return "MAKE_PROBE";
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
        case Opcode::ACCESS_PROPERTY:
            return "ACCESS_PROPERTY";
        case Opcode::LOAD_GLOBAL:
            return "LOAD_GLOBAL";
        case Opcode::LOAD_CONSOLE:
            return "LOAD_CONSOLE";
        case Opcode::RETURN:
            return "RETURN";
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
    std::vector<std::string> parameters;
    BoolOperator boolop;
    size_t line;
    std::string propName = "";

    explicit Instruction(Opcode op) : op(op) {}
    
    Instruction(Opcode op, size_t index) : op(op), index(index)  {}
    
    Instruction(Opcode op, int argc) : op(op), argc(argc) {}
    
    Instruction(Opcode op, const std::string& name) : op(op), name(name) {}
    
    Instruction(Opcode op, const std::vector<std::shared_ptr<Instruction>>& body, const std::vector<std::string>& parameters)
        : op(op), body(body), parameters(parameters) {}
    
    Instruction(std::string name, Opcode op, const std::vector<std::shared_ptr<Instruction>>& body)
        : name(name), op(op), body(body) {} // Probe constructor
    
    Instruction(Opcode op, BoolOperator boolop) : op(op), boolop(boolop) {}
    
    Instruction(Opcode op, size_t line, bool) : op(op), line(line) {}
};

} // namespace Probescript::VM