#include "prbtest.hpp"

std::vector<TestCase> g_tests;

Val getValTestLib()
{
    return std::make_shared<ObjectVal>(std::unordered_map<std::string, Val>(
    {
        {
            "assert",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.empty()) throw ThrowException(ArgumentError("Usage: assert(expression, message?: str)"));
                if (!args[0]->toBool()) throw ThrowException(CustomError(args.size() < 2 ? "Assertion failed" : args[1]->toString(), "AssertError"));
                return std::make_shared<UndefinedVal>();
            })
        },
        {
            "test",
            std::make_shared<NativeFnValue>([](std::vector<Val> args, EnvPtr env) -> Val
            {
                if (args.size() < 2) throw ThrowException(ArgumentError("Usage: test(name: str, fn: function)"));
                g_tests.push_back(TestCase(args[0]->toString(), [args]()
                {
                    evalCallWithFnVal(args[1], {}, std::make_shared<Env>());
                }));

                return std::make_shared<UndefinedVal>();
            })
        }
    }));
}

TypePtr getTypeTestLib()
{
    return std::make_shared<Type>(TypeKind::Module, "native module", std::make_shared<TypeVal>(std::unordered_map<std::string, TypePtr>(
    {
        {
            "assert",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "expression"), new VarDeclarationType(new StringLiteralType("Assertion failed"), "failmessage") })))
        },
        {
            "test",
            std::make_shared<Type>(TypeKind::Function, "native function", std::make_shared<TypeVal>(std::vector<VarDeclarationType*>({ new VarDeclarationType(new UndefinedLiteralType(), "name", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "fn") })))
        }
    })));
}

void runTests(std::string file)
{
    bool failed = false;
    std::string messages;

    for (const auto& test : g_tests)
    {
        try
        {
            test.fn();
        }
        catch(const ThrowException& e)
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