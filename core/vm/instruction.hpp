#pragma once
#include <string>
#include <vector>
#include <variant>
#include <memory>

namespace Probescript::VM
{

enum class Opcode
{
    // Load a constant at 'index' index
    LOAD_CONST,

    // Debug print instruction - this is never used by the compiler
    PRINT,

    // Arithmetic instructions - perform operations onn the two top items of the stack
    ADD,
    SUB,
    MUL,
    DIV,

    // Stop the VM and return prematurly
    HALT,

    // Call the top of the stack after argc arguments have been popped
    CALL,

    // Store the top of the stack as 'name' in the current scope
    STORE,

    // Load 'name' from the current scope
    LOAD,

    // Make a function with instruction->parameters parameters and instruction->body body
    MAKE_FUNCTION,
    
    MAKE_PROBE,
    COMPARE,
    JUMP_IF_FALSE,
    START_SCOPE,
    END_SCOPE,
    JUMP,
    ASSIGN,

    // Create an empty object
    CREATE_OBJECT,

    // ACCESS_PROPERTY:
    // key: instruction->property (or top of stack)
    // object: top of stack
    ACCESS_PROPERTY,

    // ASSIGN_PROPERTY:
    // key: instruction->property (or top of stack)
    // new_value: top of stack
    // object: top of stack
    ASSIGN_PROPERTY,
    LOAD_GLOBAL,

    LOAD_CONSOLE, // Special instruction for loading a console property like println
    RETURN,
    NEGATE,
    LOAD_BOOL,
    POP, // Discard the top item of the stack

    NEW,
};

inline std::string OpCodeToString(Opcode code)
{
    switch (code)
    {
        case Opcode::LOAD_CONST:
            return "LOAD_CONST";
        case Opcode::PRINT:
            return "PRINT";
        case Opcode::NEW:
            return "NEW";
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
        case Opcode::POP:
            return "POP";
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
        case Opcode::LOAD_BOOL:
            return "LOAD_BOOL";
        default:
            return "UNKNOWN";
    }
}

enum class BoolOperator
{
    EQUALS,
    NOT_EQUALS,
    GREATER,
    LESS,

    LESS_THAN_OR_EQUAL_TO,
    GREATER_THAN_OR_EQUAL_TO,

    OR,
    AND,
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
    std::string property = "";
    bool boolLiteralValue;

    explicit Instruction(Opcode op) : op(op) {}
    
    Instruction(Opcode op, size_t index) : op(op), index(index)  {}

    Instruction(Opcode op, bool boolLiteralValue) : op(op), boolLiteralValue(boolLiteralValue)  {}
    
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