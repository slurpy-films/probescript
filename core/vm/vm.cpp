#include "vm.hpp"

using namespace Probescript::VM;

extern std::unordered_map<std::string, ValuePtr> g_valueGlobals;

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

Signal Machine::run()
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
            case Opcode::LOAD_GLOBAL:
            {
                push(g_valueGlobals[instr->name]);
                break;
            }
            case Opcode::LOAD_CONSOLE:
            {
                // Here, instr->name is the name of the console property to access. 
                // If it is empty, we push the console object directly
                if (instr->name.empty())
                {
                    push(s_Console);
                    break;
                }

                // This will push a nullptr if it does not exist, but we expect the compiler to never use this instruction on anything other than
                // console.println, console.print or console.prompt
                push(s_Console->properties[instr->name]);
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

                if (a->type == ValueType::String)
                {
                    std::ostringstream stream;

                    int len = (int)b->toNum();
                    std::string string = a->toString();

                    for (int i = 0; i < len; ++i)
                    {
                        stream << string;
                    }

                    push(std::make_shared<StringVal>(stream.str()));
                    break;
                }

                throw std::runtime_error("Can only multiply numbers and strings");
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
                std::cout << pop()->toString();
                break;
            case Opcode::RETURN:
            {
                return Signal(pop());
            }
            case Opcode::CALL:
            {
                std::vector<ValuePtr> args;

                for (size_t i = 0; i < instr->argc; i++)
                {
                    args.push_back(pop());
                };

                std::reverse(args.begin(), args.end());

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
                    
                    for (size_t i = 0; i < func->parameters.size(); ++i)
                    {
                        scope->declare(func->parameters[i], (args[i] ? args[i] : s_null));
                    }

                    Machine vm(func->body, m_consts, scope);
                    
                    Signal result = vm.run();
                    if (result.type == SignalType::Return)
                    {
                        push(result.val);
                        break;
                    }
                    
                    push(std::make_shared<NullVal>());
                    break;
                }

                if (fn->type == ValueType::Probe)
                {
                    auto probe = std::static_pointer_cast<ProbeValue>(fn);

                    ScopePtr scope = std::make_shared<Scope>(probe->scope);

                    auto body = probe->body;
                    body.push_back(std::make_shared<Instruction>(Opcode::LOAD, probe->name)); // Load the run function
                    body.push_back(std::make_shared<Instruction>(Opcode::CALL, 0)); // Call the run function (TODO: Enable arguments)
                    body.push_back(std::make_shared<Instruction>(Opcode::RETURN));

                    Machine vm(body, m_consts, scope);
                    Signal result = vm.run();
                    if (result.type == SignalType::Return)
                    {
                        push(result.val);
                        break;
                    }
                    
                    push(std::make_shared<NullVal>());
                    break;
                }

                throw std::runtime_error("Cannot call a value that is not a function: " + fn->toString());
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
            case Opcode::NEGATE:
            {
                push(std::make_shared<BooleanVal>(!pop()->toBool()));
                break;
            }
            case Opcode::MAKE_FUNCTION:
            {
                push(std::make_shared<FunctionValue>(instr->body, instr->parameters, m_scope));
                break;
            }
            case Opcode::MAKE_PROBE:
            {
                push(std::make_shared<ProbeValue>(instr->name, instr->body, m_scope));
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
                        auto left = pop();
                        auto right = pop();

                        if (left->type != ValueType::Number)
                        {
                            throw std::runtime_error("Can only use '>' on numbers");
                        }

                        push(std::make_shared<BooleanVal>(left->toNum() > right->toNum()));
                        break;
                    }
                    case BoolOperator::LESS:
                    {
                        auto left = pop();
                        auto right = pop();

                        if (left->type != ValueType::Number)
                        {
                            throw std::runtime_error("Can only use '<' on numbers");
                        }

                        push(std::make_shared<BooleanVal>(left->toNum() < right->toNum()));
                        break;
                    }
                    case BoolOperator::AND:
                    {
                        auto left = pop();
                        auto right = pop();
                        
                        push(std::make_shared<BooleanVal>(left->toBool() && right->toBool()));
                        
                        break;
                    }
                    case BoolOperator::OR:
                    {
                        auto left = pop();
                        auto right = pop();
                        
                        push(std::make_shared<BooleanVal>(left->toBool() || right->toBool()));
                        
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
            case Opcode::ACCESS_PROPERTY:
            {
                std::string key = instr->property;

                if (key.empty())
                {
                    auto top = pop();
                    key = top->toString();
                }

                auto object = pop();
                push(object->properties[key]);

                break;
            }
            case Opcode::LOAD_BOOL:
            {
                push(std::make_shared<BooleanVal>(instr->boolLiteralValue));
                break;
            }
            case Opcode::HALT:
                return Signal();

            default:
                throw std::runtime_error("Unknown instruction");
        }
    }

    return Signal();
}