#include "prbtest.hpp"

#include "core/errors.hpp"

using namespace Probescript;
using namespace Probescript::Stdlib;
using namespace Probescript::Stdlib::Prbtest;

std::vector<TestCase> g_tests = {};

VM::ValuePtr Prbtest::getValTestLib()
{
    return std::make_shared<VM::ObjectVal>(std::unordered_map<std::string, VM::ValuePtr>(
    {
        {
            "assert",
            std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext> ctx) -> VM::ValuePtr
            {
                if (args.empty()) throw ThrowException(ArgumentError("Usage: assert(expression, message?: str)"));
                if (!args[0]->toBool()) throw ThrowException(CustomError(args.size() < 2 ? "Assertion failed" : args[1]->toString(), "AssertError"));
                return std::make_shared<VM::NullVal>();
            })
        },
        {
            "test",
            std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext> ctx) -> VM::ValuePtr
            {
                if (args.size() < 2) throw ThrowException(ArgumentError("Usage: test(name: str, fn: function)"));
                g_tests.push_back(TestCase(args[0]->toString(), [args, ctx]()
                {
                    VM::call(args[1], {}, ctx);
                }));

                return std::make_shared<VM::NullVal>();
            })
        }
    }));
}

Typechecker::TypePtr Prbtest::getTypeTestLib()
{
    return std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Module, "native module", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>(
    {
        {
            "assert",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("expression", Typechecker::g_anyty), std::make_shared<Typechecker::Parameter>("failmessage", Typechecker::g_strty, true) })))
        },
        {
            "test",
            std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("name", Typechecker::g_strty), std::make_shared<Typechecker::Parameter>("fn", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function")) })))
        }
    })));
}

void Prbtest::runTests(std::string file)
{
    bool failed = false;
    std::string messages;

    for (const auto& test : g_tests)
    {
        try
        {
            test.fn();
        }
        catch(const std::exception& e)
        {
            messages += test.name + ": " + e.what();
            failed = true;
        }
    }

    if (failed)
    {
        std::cerr << ConsoleColors::RED << "FAIL  " << ConsoleColors::RESET << file << "\n\n" << messages;
        exit(1);
    }
}