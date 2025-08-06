#pragma once
#include <string>
#include <vector>
#include <memory>

namespace Probescript::VM
{

enum class Opcode
{
    // Load a constant at 'index' index
    LOAD_CONST,

    // Load null
    LOAD_NULL,

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

    // Make a class with instruction->body body and the class on the top of the stack's body if instruction->extends
    MAKE_CLASS,
    
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


    MAKE_MODULE,

    NEW,

    LOAD_STDLIB, // Load a standard library module

    DEFAULT_PARAM, // Checks if 'name' is a default parameter, and if so it assigns the top of the stack to it

    EXPORT,

    // Create an empty array
    CREATE_ARRAY,

    // Push the top of the stack to the new top of the stack
    PUSH_ARRAY,

    // Throw the top of the stack as a ThrowException
    THROW,

    // Evaluate instruction->body with the top of the stack as the catch handler
    CATCH,

    SWITCH_TOP,

    // Make the function at the top of the stack an async function
    MAKE_ASYNC,

    // Await the future at the top of the stack
    AWAIT,
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
        case Opcode::ASSIGN_PROPERTY:
            return "ASSIGN_PROPERTY";
        case Opcode::LOAD_GLOBAL:
            return "LOAD_GLOBAL";
        case Opcode::LOAD_CONSOLE:
            return "LOAD_CONSOLE";
        case Opcode::RETURN:
            return "RETURN";
        case Opcode::LOAD_BOOL:
            return "LOAD_BOOL";
        case Opcode::MAKE_CLASS:
            return "MAKE_CLASS";
        case Opcode::NEGATE:
            return "NEGATE";
        case Opcode::LOAD_STDLIB:
            return "LOAD_STDLIB";
        case Opcode::MAKE_MODULE:
            return "MAKE_MODULE";
        case Opcode::EXPORT:
            return "EXPORT";
        case Opcode::LOAD_NULL:
            return "LOAD_NULL";
        case Opcode::THROW:
            return "THROW";
        case Opcode::CREATE_ARRAY:
            return "CREATE_ARRAY";
        case Opcode::PUSH_ARRAY:
            return "PUSH_TO_ARRAY";
        case Opcode::CATCH:
            return "CATCH";
        case Opcode::SWITCH_TOP:
            return "SWITCH_TOP";
        case Opcode::CREATE_OBJECT:
            return "CREATE_OBJECT";
        case Opcode::DEFAULT_PARAM:
            return "DEFAULT_PARAM";
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

    bool extends;

    explicit Instruction(Opcode op) : op(op) {}
    
    Instruction(Opcode op, size_t index) : op(op), index(index)  {}

    Instruction(Opcode op, bool boolLiteralValue) : op(op), boolLiteralValue(boolLiteralValue)  {}
    
    Instruction(Opcode op, int argc) : op(op), argc(argc) {}
    
    Instruction(Opcode op, const std::string& name) : op(op), name(name) {}
    
    Instruction(Opcode op, const std::vector<std::shared_ptr<Instruction>>& body, const std::vector<std::string>& parameters)
        : op(op), body(body), parameters(parameters) {}
    
    Instruction(std::string name, Opcode op, const std::vector<std::shared_ptr<Instruction>>& body)
        : name(name), op(op), body(body) {} // Probe constructor

    Instruction(Opcode op, std::vector<std::shared_ptr<Instruction>>& body, bool extends)
        : op(op), body(body), extends(extends) {} 

    Instruction(Opcode op, std::vector<std::shared_ptr<Instruction>>& body)
        : op(op), body(body) {} // MAKE_MODULE constructor 
    
    Instruction(Opcode op, BoolOperator boolop) : op(op), boolop(boolop) {}
    
    Instruction(Opcode op, size_t line, bool) : op(op), line(line) {}
};

inline std::string InstructionToString(const std::shared_ptr<Instruction>& instr, size_t lineNumber = 0)
{
    if (!instr) return "NULL";
    
    std::string result = std::to_string(lineNumber) + ": " + OpCodeToString(instr->op);
    
    switch (instr->op)
    {
        case Opcode::LOAD_CONST:
            result += " " + std::to_string(instr->index);
            break;
            
        case Opcode::STORE:
        case Opcode::LOAD:
        case Opcode::ASSIGN:
        case Opcode::LOAD_GLOBAL:
        case Opcode::LOAD_STDLIB:
            result += " \"" + instr->name + "\"";
            break;
            
        case Opcode::LOAD_CONSOLE:
            if (!instr->property.empty())
                result += " \"" + instr->property + "\"";
            break;
            
        case Opcode::CALL:
        case Opcode::NEW:
            result += " " + std::to_string(instr->argc);
            break;
            
        case Opcode::JUMP:
        case Opcode::JUMP_IF_FALSE:
            result += " " + std::to_string(instr->line);
            break;
            
        case Opcode::MAKE_FUNCTION:
            result += " [";
            for (size_t i = 0; i < instr->parameters.size(); ++i)
            {
                if (i > 0) result += ", ";
                result += "\"" + instr->parameters[i] + "\"";
            }
            result += "] {\n| ";
            for (size_t i = 0; i < instr->body.size(); ++i)
            {
                if (i > 0) result += "\n| ";
                result += InstructionToString(instr->body[i], i);
            }
            result += "\n}";
            break;
            
        case Opcode::MAKE_PROBE:
            result += " \"" + instr->name + "\" {\n| ";
            for (size_t i = 0; i < instr->body.size(); ++i)
            {
                if (i > 0) result += "\n| ";
                result += InstructionToString(instr->body[i], i);
            }
            result += "\n}";
            break;

        case Opcode::MAKE_CLASS:
            result += " {\n| ";
            for (size_t i = 0; i < instr->body.size(); ++i)
            {
                if (i > 0) result += "\n| ";
                result += InstructionToString(instr->body[i], i);
            }
            result += "\n}";
            break;

        case Opcode::MAKE_MODULE:
            result += " {\n| ";
            for (size_t i = 0; i < instr->body.size(); ++i)
            {
                if (i > 0) result += "\n| ";
                result += InstructionToString(instr->body[i], i);
            }
            result += "\n}";
            break;

        case Opcode::CATCH:
            result += " {\n| ";
            for (size_t i = 0; i < instr->body.size(); ++i)
            {
                if (i > 0) result += "\n| ";
                result += InstructionToString(instr->body[i], i);
            }
            result += "\n}";
            break;
            
        case Opcode::ACCESS_PROPERTY:
        case Opcode::ASSIGN_PROPERTY:
            if (!instr->property.empty())
                result += " \"" + instr->property + "\"";
            break;
            
        case Opcode::COMPARE:
            result += " ";
            switch (instr->boolop)
            {
                case BoolOperator::EQUALS:
                    result += "EQUALS";
                    break;
                case BoolOperator::NOT_EQUALS:
                    result += "NOT_EQUALS";
                    break;
                case BoolOperator::GREATER:
                    result += "GREATER";
                    break;
                case BoolOperator::LESS:
                    result += "LESS";
                    break;
                case BoolOperator::LESS_THAN_OR_EQUAL_TO:
                    result += "LESS_THAN_OR_EQUAL_TO";
                    break;
                case BoolOperator::GREATER_THAN_OR_EQUAL_TO:
                    result += "GREATER_THAN_OR_EQUAL_TO";
                    break;
                case BoolOperator::OR:
                    result += "OR";
                    break;
                case BoolOperator::AND:
                    result += "AND";
                    break;
            }
            break;
            
        case Opcode::LOAD_BOOL:
            result += " " + std::string(instr->boolLiteralValue ? "true" : "false");
            break;
            
        default:
            break;
    }
    
    return result;
}

} // namespace Probescript::VM
