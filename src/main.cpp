#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "runtime/interpreter.hpp"
#include "ast.hpp"
#include "parser.hpp"
#include "REPL.hpp"
#include "config.hpp"
#include <fstream>
#include "modules.hpp"
#include <filesystem>

void showHelp(char* argv[]) {
    std::cerr << "ProbeScript CLI\n"
              << "Usage:\n"
              << "  " << argv[0] << " [command] [args]\n\n"
              << "Available Commands:\n"
                << "  run     Run a ProbeScript file\n"
                << "  repl    Start the ProbeScript REPL\n"
                << "  help    Shows this help menu\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        showHelp(argv);
        return 0;
    }

    if (std::string(argv[1]) == "repl") {
        REPL* repl;
        repl->start();
        delete repl;
        return 0;
    } else if (std::string(argv[1]) == "run") {
        if (argc < 3) {
            std::cerr << "Run command expects 1 argument, " << argc - 2 << " given";
            exit(1);
        }
        std::string fileName = argv[2];
        if (!std::filesystem::exists(fileName)) {
            std::cerr << "Module " << fileName << " not found";
            exit(1);
        }
        Parser parser;
        Env* env = new Env();

        std::ifstream stream(fileName);
        std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        Config::Config* config = new Config::Config(Config::Normal, "Main");

        std::pair<std::unordered_map<std::string, fs::path>, Val> indexedPair = indexModules(fileName);
        config->modules = indexedPair.first;
        config->project = indexedPair.second;
        
        ProgramType* program = parser.produceAST(file);
        Val result = eval(program, env, config);

        delete program;
    } else if (std::string(argv[1]) == "help") {
        showHelp(argv);
    } else {
        std::cerr << "Unknown command: " << argv[1];
        std::cerr << "\nUse " << argv[0] << " help to see commands";
        exit(1);
    }
}