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
            std::shared_ptr<ProgramType> program = parser.parse(src);

            Val result = eval(program, env, context);

            std::cout << result->toConsole() << "\n";
        }
        catch (const std::runtime_error& err)
        {
            std::cerr << err.what();
        }
        catch (const ThrowException& err)
        {
            std::cerr << err.what();
        }
    }
}