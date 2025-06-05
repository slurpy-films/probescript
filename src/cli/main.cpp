#include "main.hpp"

#define __PROBESCRIPTVERSION__ "1.0"

void showHelp(char* argv[])
{
    std::cout << ConsoleColors::CYAN << "Probescript v" << __PROBESCRIPTVERSION__ << "\n" << ConsoleColors::RESET
              << "Usage:\n"
              << ConsoleColors::YELLOW << "  " << argv[0] << " [command] [args]\n\n" << ConsoleColors::RESET
              << "Available Commands:\n"
                << ConsoleColors::BLUE <<  "  run     Run a probescript file\n" << ConsoleColors::RESET
                << ConsoleColors::GREEN << "  repl    Start the probescript REPL\n" << ConsoleColors::RESET
                << ConsoleColors::RED <<   "  help    Shows this help menu\n" << ConsoleColors::RESET
                << ConsoleColors::YELLOW <<  "  init    Initialize a new probescript project\n" << ConsoleColors::RESET;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        showHelp(argv);
        return 0;
    }

    if (std::string(argv[1]) == "repl")
    {
        REPL repl;
        repl.start();
        return 0;
    } else if (std::string(argv[1]) == "run")
    {
        if (argc < 3)
        {
            std::cerr << "Run command expects 1 argument, " << argc - 2 << " given";
            exit(1);
        }
        std::filesystem::path fileName = argv[2];
        if (!std::filesystem::exists(fileName))
        {
            std::cerr << "Module " << fileName << " not found";
            exit(1);
        }
        Parser parser;
        std::pair<std::unordered_map<std::string, fs::path>, Val> indexedPair = indexModules(fileName);
        EnvPtr env = std::make_shared<Env>();

        if (std::filesystem::is_directory(fileName) && indexedPair.second->properties.find("main") != indexedPair.second->properties.end())
        {
            fileName = fileName / indexedPair.second->properties["main"]->toString();
        }

        std::ifstream stream(fileName);
        std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        std::shared_ptr<Context> context = std::make_shared<Context>(RuntimeType::Normal, "Main");

        context->filename = std::filesystem::absolute(fileName).string();
        context->file = file;
        context->modules = indexedPair.first;
        context->project = indexedPair.second;
        
        ProgramType* program = parser.produceAST(file, context);

        TC tc;
        tc.checkProgram(program, std::make_shared<TypeEnv>(), context);

        Val result = eval(program, env, context);

        delete program;
    } else if (std::string(argv[1]) == "help")
    {
        showHelp(argv);
    } else if (std::string(argv[1]) == "init") 
    {
        Initializer i;
        i.start(argv);
    } else
    {
        std::cerr << "Unknown command: " << argv[1];
        std::cerr << "\nUse " << argv[0] << " help to see commands";
        exit(1);
    }
}