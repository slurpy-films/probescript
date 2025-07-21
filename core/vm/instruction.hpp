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
    PRINT, // Debug print instruction - this is never used in the standard compiler
    ADD,
    SUB,
    MUL,
    DIV,
    HALT,
    CALL,
    STORE,
    LOAD,
    MAKE_FUNCTION,
};

struct Instruction
{
    Opcode op;

    Instruction(Opcode op)
        : op(op) {}
};

struct LOAD_CONST : public Instruction
{
    size_t index;

    LOAD_CONST(size_t index)
        : Instruction(Opcode::LOAD_CONST), index(index) {}
};

struct CALL : public Instruction
{
    int argc;

    CALL(int argc)
        : Instruction(Opcode::CALL), argc(argc) {}
};

struct STORE : public Instruction
{
    std::string name;

    STORE(std::string name)
        : Instruction(Opcode::STORE), name(name) {}
};

struct LOAD : public Instruction
{
    std::string name;

    LOAD(std::string name)
        : Instruction(Opcode::LOAD), name(name) {}
};

struct MAKE_FUNCTION : public Instruction {
    std::vector<std::shared_ptr<Instruction>> body;

    MAKE_FUNCTION(const std::vector<std::shared_ptr<Instruction>>& body)
        : Instruction(Opcode::MAKE_FUNCTION), body(body) {}
};

}