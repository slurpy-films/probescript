#include "vm.hpp"
#include "utils.hpp"

#include <memory>
#include <algorithm>

using namespace Probescript;
using namespace Probescript::VM;

extern std::unordered_map<std::string, ValuePtr> g_valueGlobals;
extern std::unordered_map<std::string, ValuePtr> g_valueStdlib;

ValuePtr VM::call(ValuePtr fn, std::vector<ValuePtr> args, std::shared_ptr<FunctionContext> context)
{
    if (fn->type == ValueType::Function)
    {
        auto func = std::static_pointer_cast<FunctionValue>(fn);

        auto scope = std::make_shared<Scope>(func->scope);

        size_t size = func->parameters.size();
        size_t argc = args.size();

        for (size_t i = 0; i < size; ++i)
        {
            scope->declare(func->parameters[i], argc <= i ? std::make_shared<NullVal>(true) : args[i]);
        }

        auto constants = context->constants;
        std::shared_ptr<Machine> vm = std::make_shared<Machine>(func->body, constants, scope);
        vm->setGlobals(context->globals);

        if (func->async)
        {
            auto fut = std::async(std::launch::async, [func, vm]() -> ValuePtr
            {
                Signal result = vm->run();
                if (result.type == SignalType::Return)
                {
                    return result.val;
                }
                
                return std::make_shared<NullVal>();
            });

            return std::make_shared<FutureVal>(fut.share());
        }
        
        Signal result = vm->run();
        if (result.type == SignalType::Return)
        {
            return result.val;
        }

        return std::make_shared<NullVal>();
    }

    if (fn->type == ValueType::NativeFunction)
    {
        return std::static_pointer_cast<NativeFunctionVal>(fn)->call(args, context);
    }

    throw std::runtime_error("Cannot call a value that is not a function\n");
}

ValuePtr Machine::pop()
{
    if (m_stack.empty())
    {
        throw std::runtime_error("Stack underflow\n"); 
    }

    ValuePtr last = m_stack.back();
    m_stack.pop_back();
    return last;
}

void Machine::push(ValuePtr val)
{
    m_stack.push_back(val);
}

void Machine::registerGlobal(const std::string& name, ValuePtr value)
{
    m_globals[name] = value;
}

std::unordered_map<std::string, ValuePtr> Machine::getGlobals()
{
    return m_globals;
}

void Machine::setGlobals(std::unordered_map<std::string, ValuePtr> globals)
{
    m_globals = globals;
}

const std::vector<ValuePtr>& Machine::getConstants()
{
    return m_consts;
}

ValuePtr Machine::lookup(std::string name)
{
    return m_scope->lookupVar(name);
}

void Machine::load(const std::vector<std::shared_ptr<Instruction>>& bytecode, std::vector<ValuePtr> consts)
{
    m_bytecode = bytecode;
    m_consts = consts;
}

Signal Machine::runInstruction(std::shared_ptr<Instruction> instr)
{
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
            push(m_globals[instr->name]);
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

            // This will terminate the process if it fails,
            // but we expect the compiler to never use this instruction on anything other than
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

            throw std::runtime_error("Can only add strings and numbers\n");
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

            throw std::runtime_error("Can only subract numbers\n");
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

            throw std::runtime_error("Can only multiply numbers and strings\n");
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

            throw std::runtime_error("Can only divide numbers\n");
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
                push(std::static_pointer_cast<NativeFunctionVal>(fn)->call(args, std::make_shared<FunctionContext>(m_consts, m_globals)));
                break;
            }

            if (fn->type == ValueType::Function)
            {
                static auto defaultNull = std::make_shared<NullVal>(true);

                auto func = std::static_pointer_cast<FunctionValue>(fn);

                ScopePtr scope = std::make_shared<Scope>(func->scope);
                
                for (size_t i = 0; i < func->parameters.size(); ++i)
                {
                    scope->declare(func->parameters[i], (args.size() > i ? args[i] : defaultNull));
                }

                std::shared_ptr<Machine> vm = std::make_shared<Machine>(func->body, m_consts, scope);
                vm->setGlobals(m_globals);

                if (func->async)
                {
                    auto fut = std::async(std::launch::async, [func, vm]() -> ValuePtr
                    {
                        Signal result = vm->run();
                        if (result.type == SignalType::Return)
                        {
                            return result.val;
                        }
                        
                        return std::make_shared<NullVal>();
                    });

                    push(std::make_shared<FutureVal>(fut.share()));

                    break;
                }
                
                Signal result = vm->run();
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
                vm.setGlobals(m_globals);

                Signal result = vm.run();
                
                if (result.type == SignalType::Return)
                {
                    push(result.val);
                    break;
                }
                
                push(std::make_shared<NullVal>());
                break;
            }

            throw std::runtime_error("Cannot call a value that is not a function or a probe: " + fn->toString() + "\n");
            break;
        }
        case Opcode::AWAIT:
        {
            auto fut = pop();
            if (fut->type != ValueType::Future)
            {
                throw std::runtime_error("Can only await futures\n");
            }

            push(std::static_pointer_cast<FutureVal>(fut)->future.get());
            break;
        }
        case Opcode::NEW:
        {
            std::vector<ValuePtr> args;

            for (size_t i = 0; i < instr->argc; i++)
            {
                args.push_back(pop());
            };

            std::reverse(args.begin(), args.end());

            ValuePtr cls = pop();

            if (cls->type == ValueType::NativeClass)
            {
                push(std::static_pointer_cast<NativeClassVal>(cls)->call(args));
                break;
            }

            if (cls->type == ValueType::Class)
            {
                auto castedClass = std::static_pointer_cast<ClassVal>(cls);
                auto scope = std::make_shared<Scope>(castedClass->scope);
                auto thisObj = std::make_shared<ObjectVal>();

                scope->declare("this", thisObj);
                
                size_t bodySize = castedClass->body.size();
                for (size_t i = 0; i < bodySize; ++i)
                {
                    auto instr = castedClass->body[i];

                    switch (instr->op)
                    {
                        case Opcode::MAKE_FUNCTION:
                        {
                            thisObj->properties[instr->name] = std::make_shared<FunctionValue>(instr->body, instr->parameters, scope);

                            // The compiler will generate a STORE instruction to store the function, but we do not want that
                            if (castedClass->body[i + 1]->op == Opcode::STORE)
                            {
                                ++i;
                            }

                            break;
                        }
                        case Opcode::STORE:
                        {
                            thisObj->properties[instr->name] = pop();
                            break;
                        }

                        default:
                        {
                            runInstruction(instr);
                        }
                    }
                }

                if (thisObj->properties.find("new") != thisObj->properties.end())
                {
                    call(thisObj->properties["new"], args, std::make_shared<FunctionContext>(m_consts, m_globals));
                }

                push(thisObj);
                break;
            }

            throw std::runtime_error("Cannot construct a value that is not a class: " + cls->toString() + "\n");
            break;
        }
        case Opcode::STORE:
        {
            push(m_scope->declare(instr->name, pop()));
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
        case Opcode::MAKE_ASYNC:
        {
            auto fn = pop();

            // This should never happen, but we add a check just in case
            if (fn->type != ValueType::Function)
            {
                throw std::runtime_error("Can only make functions async");
            }

            auto castedFn = std::static_pointer_cast<FunctionValue>(fn);
            castedFn->async = true;
            push(castedFn);
            break;
        }
        case Opcode::MAKE_PROBE:
        {
            push(std::make_shared<ProbeValue>(instr->name, instr->body, m_scope));
            break;
        }
        case Opcode::POP:
        {
            pop();
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
                case BoolOperator::NOT_EQUALS:
                {
                    push(std::make_shared<BooleanVal>(!pop()->compare(pop())));
                    break;
                }
                case BoolOperator::GREATER:
                {
                    auto left = pop();
                    auto right = pop();

                    if (left->type != ValueType::Number)
                    {
                        throw std::runtime_error("Can only use '>' on numbers\n");
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
                        throw std::runtime_error("Can only use '<' on numbers\n");
                    }

                    push(std::make_shared<BooleanVal>(left->toNum() < right->toNum()));
                    break;
                }
                case BoolOperator::AND:
                {
                    auto right = pop();
                    auto left = pop();
                    
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
                case BoolOperator::GREATER_THAN_OR_EQUAL_TO:
                {
                    auto left = pop();
                    auto right = pop();

                    if (left->type != ValueType::Number)
                    {
                        throw std::runtime_error("Can only use '>=' on numbers\n");
                    }

                    push(std::make_shared<BooleanVal>(left->toNum() >= right->toNum()));
                    break;
                }
                case BoolOperator::LESS_THAN_OR_EQUAL_TO:
                {
                    auto left = pop();
                    auto right = pop();

                    if (left->type != ValueType::Number)
                    {
                        throw std::runtime_error("Can only use '<=' on numbers\n");
                    }

                    push(std::make_shared<BooleanVal>(left->toNum() <= right->toNum()));
                    break;
                }

                default:
                    throw std::runtime_error("Unknown comparison operator\n");
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
        case Opcode::LOAD_NULL:
        {
            static auto null = std::make_shared<NullVal>();
            push(null);
            break;
        }
        case Opcode::START_SCOPE:
        {
            m_scope = std::make_shared<Scope>(m_scope);
            ++m_scopeCount;
            break;
        }
        case Opcode::END_SCOPE:
        {
            m_scope = m_scope->getParent();
            break;
        }
        case Opcode::ACCESS_PROPERTY:
        {
            static auto null = std::make_shared<NullVal>();
            std::string key = instr->property;

            if (key.empty())
            {
                auto top = pop();
                key = top->toString();
            }

            auto object = pop();
            push(object->properties.find(key) != object->properties.end() ? object->properties[key] : null);

            break;
        }
        case Opcode::ASSIGN_PROPERTY:
        {
            std::string key = instr->property;

            if (key.empty()) // If the key is empty, we are working with a computed property
            {
                auto top = pop();
                key = top->toString();
            }

            auto value = pop();
            auto object = pop();
            object->properties[key] = value;

            push(object->properties[key]);

            break;
        }
        case Opcode::LOAD_BOOL:
        {
            push(std::make_shared<BooleanVal>(instr->boolLiteralValue));
            break;
        }
        case Opcode::CREATE_OBJECT:
        {
            push(std::make_shared<ObjectVal>()); // Simply push an empty object
            break;
        }
        case Opcode::DEFAULT_PARAM:
        {
            ValuePtr val = pop();
            ValuePtr null = m_scope->lookupVar(instr->name);

            if (null->type == ValueType::Null && std::static_pointer_cast<NullVal>(null)->isDefaultParam)
            {
                m_scope->assign(instr->name, val);
            }

            break;
        }
        case Opcode::LOAD_STDLIB:
        {
            push(g_valueStdlib[instr->name]);
            break;
        }
        case Opcode::MAKE_MODULE:
        {
            Machine vm(instr->body, m_consts, std::make_shared<Scope>());
            vm.setGlobals(m_globals);

            Signal result = vm.run();

            push(std::make_shared<ObjectVal>(result.exports));
            break;
        }
        case Opcode::MAKE_CLASS:
        {
            std::vector<std::shared_ptr<Instruction>> body;
            if (instr->extends)
            {
                auto parent = pop(); // The class parent should be at the top of the stack
                if (parent->type != ValueType::Class)
                {
                    throw std::runtime_error("Cannot inherit from non-class value\n");
                }
                body = std::static_pointer_cast<ClassVal>(parent)->body;
                for (const auto& instr : instr->body)
                {
                    body.push_back(instr);
                }
            }
            else
            {
                body = instr->body;
            }

            push(std::make_shared<ClassVal>(body, m_scope));
            break;
        }
        case Opcode::EXPORT:
        {
            auto value = pop();
            m_exports[instr->name] = value;

            break;
        }
        case Opcode::CREATE_ARRAY:
        {
            push(std::make_shared<ArrayVal>());
            break;
        }
        case Opcode::PUSH_ARRAY:
        {
            auto val = pop();
            auto array = pop();

            if (array->type != ValueType::Array)
            {
                throw std::runtime_error("Cannot push to non-array");
            }

            std::static_pointer_cast<ArrayVal>(array)->items.push_back(val);
            break;
        }
        case Opcode::THROW:
        {
            auto err = pop();
            throw ThrowException(err);
            break; // Break for good measure
        }
        case Opcode::CATCH:
        {
            auto catcher = pop();
            size_t scopeCount = m_scopeCount;

            m_scope = std::make_shared<Scope>(m_scope);
            try
            {
                for (const auto& instruction : instr->body)
                {
                    runInstruction(instruction);
                }
            }
            catch(const ThrowException& e)
            {
                // First ensure scope cleanup
                while (m_scopeCount > scopeCount && m_scope && m_scope->getParent()) {
                    m_scope = m_scope->getParent();
                    --m_scopeCount;
                }

                // Then we can call catch
                call(catcher, { e.getValue() }, std::make_shared<FunctionContext>(m_consts, m_globals));
            }
            
            m_scope = m_scope->getParent();

            break;
        }
        case Opcode::SWITCH_TOP:
        {
            auto top = pop();
            auto newTop = pop();

            m_stack.push_back(top);
            m_stack.push_back(newTop);

            break;
        }
        case Opcode::HALT:
            return Signal();

        default:
            throw std::runtime_error("Unknown instruction\n");
    }

    return Signal();
}

Signal Machine::run()
{
    auto lastEval = Signal();
    while (ip < m_bytecode.size())
    {
        lastEval = runInstruction(m_bytecode[ip++]);
        if (lastEval.type == SignalType::Return)
        {
            return lastEval;
        }
    }

    auto s = Signal();
    s.val = m_stack.empty() ? std::make_shared<NullVal>() : pop();
    s.exports = m_exports;
    return s;
}
