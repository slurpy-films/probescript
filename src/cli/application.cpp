#include "application.hpp"

using namespace Probescript;

fs::path g_currentCwd = std::filesystem::current_path();

// For webassembly
extern "C"
{
    const char* interpret(const char* raw)
    {
        try
        {
            std::string code = std::string(raw);
            std::shared_ptr<AST::ProgramType> program = Parser().parse(code);
            Interpreter::eval(program, std::make_shared<Env>());

            return "";
        }
        catch (const std::runtime_error& err)
        {
            return err.what();
        }
        catch (const ThrowException& err)
        {
            return err.what();
        }
    }
}

void showHelp(char* argv[])
{
    std::cout << ConsoleColors::CYAN << "Probescript v" << __PROBESCRIPTVERSION__ << "\n" << ConsoleColors::RESET
              << "Usage:\n"
              << ConsoleColors::YELLOW << "  probescript " << ConsoleColors::RESET << ConsoleColors::GREEN << "[command] [args]\n\n" << ConsoleColors::RESET
              << "Available Commands:\n"
                << ConsoleColors::BLUE << "  run " << ConsoleColors::RESET << "  Run a probescript file\n"
                << ConsoleColors::BLUE << "  repl" << ConsoleColors::RESET << "  Start the probescript REPL\n"
                << ConsoleColors::BLUE << "  test" << ConsoleColors::RESET << "  Run tests on a probescript file using the 'prbtest' standard library\n"
                << ConsoleColors::BLUE << "  init" << ConsoleColors::RESET << "  Initialize a probescript project\n";
}

Application::Application(int argc, char* argv[])
{
    m_argv = argv;
    for (size_t i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        if (arg.find("--") == 0 || arg.find("-") == 0)
        {
            m_flags.push_back(arg);
        }
        else if (m_command.empty())
        {
            m_command = arg;
        }
        else
        {
            m_args.push_back(arg);
        }
    }
}

void Application::run()
{
    if (m_command == "repl")
    {
        REPL repl;
        repl.start();
        return;
    } else if (m_command == "run")
    {
        if (m_args.empty())
        {
            std::cerr << "Run command expects 1 argument, 0 given";
            exit(1);
        }

        fs::path fileName(m_args[0]);
        try
        {
            Parser parser;
            std::pair<std::unordered_map<std::string, fs::path>, Values::Val> indexedPair = ModuleIndexer::indexModules(fileName);
            EnvPtr env = std::make_shared<Env>();

            if (std::filesystem::is_directory(fileName) && indexedPair.second->properties.find("main") != indexedPair.second->properties.end())
            {
                fileName = fileName / indexedPair.second->properties["main"]->toString();
            }

            std::ifstream stream(fileName);
            std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

            std::shared_ptr<Context> context = std::make_shared<Context>(RuntimeType::Normal, "Main");

            g_currentCwd = std::filesystem::absolute(fileName).parent_path();

            context->filename = std::filesystem::absolute(fileName).string();
            context->file = file;
            context->modules = indexedPair.first;
            context->project = indexedPair.second;
            
            std::shared_ptr<AST::ProgramType> program = parser.parse(file, context);

            std::shared_ptr<Typechecker::TypeEnv> typeenv = std::make_shared<Typechecker::TypeEnv>();


            Typechecker::TC tc;
            tc.checkProgram(program, typeenv, context);

            Values::Val result = Interpreter::eval(program, env, context);

            return;
        }
        catch (const std::runtime_error& err)
        {
            std::cerr << err.what();
            exit(1);
        }
        catch (const ThrowException& err)
        {
            std::cerr << err.what();
            exit(1);
        }
    }
    else if (m_command == "test")
    {
        if (m_args.empty())
        {
            std::cerr << "Run command expects 1 argument, 0 given";
            exit(1);
        }

        fs::path fileName(m_args[0]);
        try
        {
            Parser parser;
            std::pair<std::unordered_map<std::string, fs::path>, Values::Val> indexedPair = ModuleIndexer::indexModules(fileName);
            EnvPtr env = std::make_shared<Env>();

            if (std::filesystem::is_directory(fileName) && indexedPair.second->properties.find("main") != indexedPair.second->properties.end())
            {
                fileName = fileName / indexedPair.second->properties["main"]->toString();
            }

            std::ifstream stream(fileName);
            std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

            std::shared_ptr<Context> context = std::make_shared<Context>(RuntimeType::Normal, "Main");

            g_currentCwd = std::filesystem::absolute(fileName).parent_path();

            context->filename = std::filesystem::absolute(fileName).string();
            context->file = file;
            context->modules = indexedPair.first;
            context->project = indexedPair.second;
            
            std::shared_ptr<AST::ProgramType> program = parser.parse(file, context);

            std::shared_ptr<Typechecker::TypeEnv> typeenv = std::make_shared<Typechecker::TypeEnv>();

            Typechecker::TC tc;
            tc.checkProgram(program, typeenv, context);
        

            Values::Val result = Interpreter::eval(program, env, context);

            Stdlib::Prbtest::runTests(fileName.string());
        }
        catch (const std::runtime_error& err)
        {
            std::cerr << err.what();
            exit(1);
        }
        catch (const ThrowException& err)
        {
            std::cerr << err.what();
            exit(1);
        }
    }
    else if (std::find(m_flags.begin(), m_flags.end(), "-h") != m_flags.end() || std::find(m_flags.begin(), m_flags.end(), "--help") != m_flags.end()) 
        showHelp(m_argv);
    else if (std::find(m_flags.begin(), m_flags.end(), "-v") != m_flags.end() || std::find(m_flags.begin(), m_flags.end(), "--version") != m_flags.end()) 
        std::cout << "v" << __PROBESCRIPTVERSION__ << "\n";
    else if (m_command == "init") 
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

        std::cout << "Project initialized! Run it with " << m_argv[0] << " run " << name << "\n";
    } else
    {
        std::cerr << "Unknown command: " << m_command;
        std::cerr << "\nRun 'probescript --help' to see commands\n";
        exit(1);
    }
}