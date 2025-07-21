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

void Machine::runLoadConst(std::shared_ptr<LOAD_CONST> loadConst)
{
    if (loadConst->index >= m_consts.size())
        throw std::runtime_error("Invalid constant index: " + std::to_string(loadConst->index) + " constant pool size: " + std::to_string(m_consts.size()));

    push(m_consts[loadConst->index]);
}

void Machine::runPrint()
{
    std::cout << pop()->toString() << "\n";
}

void Machine::runCall(std::shared_ptr<CALL> call)
{
    std::vector<ValuePtr> args;

    for (size_t i = 0; i < call->argc; i++)
    {
        args.push_back(pop());
    };

    ValuePtr fn = pop();

    if (fn->type == ValueType::NativeFunction)
    {
        push(std::static_pointer_cast<NativeFunctionVal>(fn)->call(args));
        return;
    }

    if (fn->type == ValueType::Function)
    {
        auto func = std::static_pointer_cast<FunctionValue>(fn);
        ScopePtr scope = std::make_shared<Scope>(m_scope);
        
        Machine vm(func->body, m_consts, scope);
        vm.run();

        return;
    }

    throw std::runtime_error("Cannot call a value that is not a function");
}

void Machine::runStore(std::shared_ptr<STORE> store)
{
    m_scope->declareVar(store->name, pop());
}

void Machine::runLoad(std::shared_ptr<LOAD> load)
{
    push(m_scope->lookupVar(load->name));
}

void Machine::run()
{
    while (ip < m_bytecode.size())
    {
        std::shared_ptr<Instruction> instr = m_bytecode[ip++];
        switch (instr->op)
        {
            case Opcode::LOAD_CONST:
            {
                auto loadConst = std::static_pointer_cast<LOAD_CONST>(instr);
                runLoadConst(loadConst);
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
                runPrint();
                break;
            case Opcode::CALL:
            {
                runCall(std::static_pointer_cast<CALL>(instr));
                break;
            }
            case Opcode::STORE:
            {
                runStore(std::static_pointer_cast<STORE>(instr));
                break;
            }
            case Opcode::LOAD:
            {
                runLoad(std::static_pointer_cast<LOAD>(instr));
                break;
            }
            case Opcode::HALT:
                return;

            case Opcode::MAKE_FUNCTION:
            {
                auto fn = std::static_pointer_cast<MAKE_FUNCTION>(instr);

                push(std::make_shared<FunctionValue>(fn->body, m_scope));
                break;
            }

            default:
                throw std::runtime_error("Unknown instruction");
        }
    }
}