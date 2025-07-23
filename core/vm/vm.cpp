#include "vm.hpp"

using namespace Probescript::VM;

ValuePtr Machine::pop()
{
    if (m_stack.empty())
    {
        throw std::runtime_error("Stack underflow"); 
    }

    ValuePtr last = m_stack.back();
    m_stack.pop_back();
    return last;
}

void Machine::push(ValuePtr val)
{
    m_stack.push_back(val);
}

void Machine::run()
{
    while (ip < m_bytecode.size())
    {
        Instruction* instr = m_bytecode[ip++].get();

        switch (instr->op)
        {
            case Opcode::LOAD_CONST:
            {
                if (instr->index >= m_consts.size())
                {
                    throw std::runtime_error("Invalid constant index: " + std::to_string(instr->index) + ". Constant pool size: " + std::to_string(m_consts.size()));
                }

                push(m_consts[instr->index]);
                break;
            }
            case Opcode::ADD:
            {
                ValuePtr a = pop();
                ValuePtr b = pop();

                if (a->type == ValueType::String)
                {
                    push(std::make_shared<StringVal>(a->toString() + b->toString()));
                    break;
                }

                if (a->type == ValueType::Number)
                {
                    push(std::make_shared<NumberVal>(a->toNum() + b->toNum()));
                    break;
                }

                throw std::runtime_error("Can only add strings and numbers");
            }
            case Opcode::SUB:
            {
                ValuePtr a = pop();
                ValuePtr b = pop();

                if (a->type == ValueType::Number)
                {
                    push(std::make_shared<NumberVal>(a->toNum() - b->toNum()));
                    break;
                }

                throw std::runtime_error("Can only subract numbers");
            }
            case Opcode::MUL:
            {
                ValuePtr a = pop();
                ValuePtr b = pop();

                if (a->type == ValueType::Number)
                {
                    push(std::make_shared<NumberVal>(a->toNum() * b->toNum()));
                    break;
                }

                throw std::runtime_error("Can only multiply numbers");
            }
            case Opcode::DIV:
            {
                ValuePtr a = pop();
                ValuePtr b = pop();

                if (a->type == ValueType::Number)
                {
                    push(std::make_shared<NumberVal>(a->toNum() / b->toNum()));
                    break;
                }

                throw std::runtime_error("Can only divide numbers");
            }
            case Opcode::PRINT:
                std::cout << pop()->toString() << "\n";
                break;
            case Opcode::CALL:
            {
                std::vector<ValuePtr> args;

                for (size_t i = 0; i < instr->argc; i++)
                {
                    args.push_back(pop());
                };

                ValuePtr fn = pop();

                if (fn->type == ValueType::NativeFunction)
                {
                    push(std::static_pointer_cast<NativeFunctionVal>(fn)->call(args));
                    break;
                }

                if (fn->type == ValueType::Function)
                {
                    auto func = std::static_pointer_cast<FunctionValue>(fn);
                    ScopePtr scope = std::make_shared<Scope>(m_scope);
                    
                    Machine vm(func->body, m_consts, scope);
                    vm.run();

                    break;
                }

                throw std::runtime_error("Cannot call a value that is not a function");
                break;
            }
            case Opcode::STORE:
            {
                m_scope->declare(instr->name, pop());
                break;
            }
            case Opcode::ASSIGN:
            {
                push(m_scope->assign(instr->name, pop()));
                break;
            }
            case Opcode::LOAD:
            {
                push(m_scope->lookupVar(instr->name));
                break;
            }
            case Opcode::MAKE_FUNCTION:
            {
                push(std::make_shared<FunctionValue>(instr->body, m_scope));
                break;
            }
            case Opcode::COMPARE:
            {
                switch (instr->boolop)
                {
                    case BoolOperator::EQUALS:
                    {
                        push(std::make_shared<BooleanVal>(pop()->compare(pop())));
                        break;
                    }
                    case BoolOperator::GREATER:
                    {
                        auto top = pop();
                        if (top->type != ValueType::Number)
                        {
                            throw std::runtime_error("Can only use '>' on numbers");
                        }

                        push(std::make_shared<BooleanVal>(top->toNum() > pop()->toNum()));

                        break;
                    }
                    case BoolOperator::LESS:
                    {
                        auto top = pop();
                        if (top->type != ValueType::Number)
                        {
                            throw std::runtime_error("Can only use '<' on numbers");
                        }

                        push(std::make_shared<BooleanVal>(top->toNum() < pop()->toNum()));

                        break;
                    }

                    default:
                        throw std::runtime_error("Unknown comparison operator");
                }

                break;
            }
            case Opcode::JUMP_IF_FALSE:
            {
                if (!pop()->toBool())
                {
                    ip = instr->line;
                }

                break;
            }
            case Opcode::JUMP:
            {
                ip = instr->line;
                break;
            }
            case Opcode::START_SCOPE:
            {
                m_scope = std::make_shared<Scope>(m_scope);
                break;
            }
            case Opcode::END_SCOPE:
            {
                m_scope = m_scope->getParent();
                break;
            }
            case Opcode::HALT:
                return;

            default:
                throw std::runtime_error("Unknown instruction");
        }
    }
}