#include "repl.hpp"

void REPL::start()
{
    std::cout << "REPL v1.0\n";
    EnvPtr env = std::make_shared<Env>();
    TypeEnvPtr typeenv = std::make_shared<TypeEnv>();
    std::shared_ptr<Context> context = std::make_shared<Context>(RuntimeType::REPL);

    while (true)
    {
        std::cout << "> ";
        Parser parser;
        std::string src;
        std::getline(std::cin, src);

        if (src.find("exit") == 0) break;
        try
        {
            ProgramType* program = parser.parse(src);

            TC tc;
            tc.checkProgram(program, typeenv, context);

            Val result = eval(program, env, context);

            std::cout << result->toConsole() << "\n";
            delete program;
        }
        catch (const std::runtime_error& err)
        {
            std::cerr << err.what();
        }
    }
}