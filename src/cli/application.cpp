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
                << ConsoleColors::BLUE << "  init" << ConsoleColors::RESET << "  initialise a probescript project\n";
}

Application::Application(int argc, char* argv[])
{
    m_argv = argv;
    for (size_t i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        if (arg.find("--") == 0 || arg.find("-") == 0)
        {
            m_flags.insert(arg);
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
    std::ios::sync_with_stdio(false);

    // Start the flush thread
    std::thread([]()
    {
        while (true)
        {
            std::cout << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
    
    if (m_command == "repl")
    {
        REPL repl;
        repl.start();
        return;
    }
    else if (m_command == "run")
    {
        std::vector<std::shared_ptr<VM::Instruction>> instructions;
        std::vector<VM::ValuePtr> constants;

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
                std::pair<std::unordered_map<std::string, fs::path>, VM::ValuePtr> indexedPair = ModuleIndexer::indexModules(fileName);

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

                // Perform typechecks
                Typechecker::TC typechecker;
                typechecker.checkProgram(program, std::make_shared<Typechecker::TypeEnv>(), context);
                
                Compiler compiler(program, context);
                compiler.compile();

                instructions = compiler.getInstructions();
                constants = compiler.getConstants();
            }
            catch (const std::exception& err)
            {
                std::cerr << err.what();
                exit(1);
            }
        }

        // If the -l flag is present, log out all the instructions
        if (m_flags.count("-l"))
        {
            size_t lineNumber = 0;

            for (const auto& instr : instructions)
            {
                std::cout << VM::InstructionToString(instr, lineNumber++) << "\n";
            }
        }

        VM::Machine vm(instructions, constants, std::make_shared<VM::Scope>());
        
        try
        {
            vm.run();
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::flush;
            exit(1);
        }
    }
    else if (m_command == "test")
    {
        if (m_args.empty())
        {
            std::cerr << "'test' command expects 1 argument, 0 given";
            exit(1);
        }
        fs::path fileName(m_args[0]);

        std::vector<std::shared_ptr<VM::Instruction>> instructions;
        std::vector<VM::ValuePtr> constants;

        {
            try
            {
                Parser parser;
                std::pair<std::unordered_map<std::string, fs::path>, VM::ValuePtr> indexedPair = ModuleIndexer::indexModules(fileName);

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

                // Perform typechecks
                Typechecker::TC typechecker;
                typechecker.checkProgram(program, std::make_shared<Typechecker::TypeEnv>(), context);
                
                Compiler compiler(program, context);
                compiler.compile();

                instructions = compiler.getInstructions();
                constants = compiler.getConstants();
            }
            catch (const std::exception& err)
            {
                std::cerr << err.what();
                exit(1);
            }
        }

        // If the -l flag is present, log out all the instructions
        if (m_flags.count("-l"))
        {
            size_t lineNumber = 0;

            for (const auto& instr : instructions)
            {
                std::cout << VM::InstructionToString(instr, lineNumber++) << "\n";
            }
        }

        VM::Machine vm(instructions, constants, std::make_shared<VM::Scope>());
        
        try
        {
            vm.run();
            Stdlib::Prbtest::runTests(fileName.string());
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::flush;
            exit(1);
        }
    }
    else if (
        m_flags.count("-h")
        || m_flags.count("--help")
    )
    { 
        showHelp(m_argv);
    }
    else if (
        m_flags.count("-v")
        || m_flags.count("--version")
    )
    { 
        std::cout << "v" << __PROBESCRIPTVERSION__ << "\n";
    }
    else if (m_command == "init") 
    {
        std::string name;
        std::string main;

        std::cout << "Probescript project initialiser\n\n";
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

        std::cout << "Project initialised! Run it with " << m_argv[0] << " run " << name << "\n";
    } else
    {
        std::cerr << "Unknown command: " << m_command;
        std::cerr << "\nRun 'probescript --help' to see commands\n";
        exit(1);
    }
}
