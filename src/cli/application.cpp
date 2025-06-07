#include "application.hpp"

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

Application::Application(int l_argc, char* l_argv[])
{
    argc = l_argc;
    argv = l_argv;
}

void Application::run()
{
    if (argc < 2)
    {
        showHelp(argv);
        return;
    }

    if (std::string(argv[1]) == "repl")
    {
        REPL repl;
        repl.start();
        return;
    } else if (std::string(argv[1]) == "run")
    {
        if (argc < 3)
        {
            std::cerr << "Run command expects 1 argument, " << argc - 2 << " given";
            exit(1);
        }

        fs::path fileName(argv[2]);

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

        std::shared_ptr<TypeEnv> typeenv = std::make_shared<TypeEnv>();

        TC tc;
        tc.checkProgram(program, typeenv, context);

        Val result = eval(program, env, context);

        delete program;

        return;
    } else if (std::string(argv[1]) == "help") showHelp(argv);
    else if (std::string(argv[1]) == "init") 
    {
        std::string name;
        std::string main;

        std::cout << "Probescript project initializer\n\n";
        std::cout << "Project name: ";
        std::getline(std::cin, name);

        std::cout << "\nMain file: (main.prb) ";
        std::getline(std::cin, main);
        if (main.empty()) main = "main.prb";

        fs::path projectFile(name + "/project.json");
        fs::path mainFile(name + "/" + main);
        fs::create_directory(name);
        
        std::ofstream outProjectFile(projectFile);
        std::ofstream outMainFile(mainFile);

        outMainFile << "probe Main {\n"
                    << "\tMain() {\n"
                    << "\t\tconsole.println(\"Hello World!\");\n"
                    << "\t}\n"
                    << "};";

        outProjectFile << "{\n\t\"name\": \"" << name << "\"" << ",\n\t\"main\": \"" << main << "\"\n}";

        outMainFile.close();
        outProjectFile.close();

        std::cout << "Project initialized! Run it with " << argv[0] << " run " << name;
    } else
    {
        std::cerr << "Unknown command: " << argv[1];
        std::cerr << "\nUse " << argv[0] << " help to see commands";
        exit(1);
    }
}