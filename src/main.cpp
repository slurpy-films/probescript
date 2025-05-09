#include "parser.hpp"
#include "runtime/interpreter.hpp"
#include "REPL.hpp"
#include "modules.hpp"
#include "config.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include "threads.hpp"
#include "stdlib/http.hpp"

std::vector<std::thread> globalThreads;

int main(int argc, char* argv[]) {
    Parser parser;
    Env* env = new Env();
    if (argc > 1) {
        std::string probe = "Main";
        std::string arg;
        for (size_t i = 0; i < argc; ++i) {
            if (argv[i] == "-P") {
                probe = argv[i + 1];
            } else arg = argv[i];
        }
        if (arg.find(".probe") == std::string::npos) {
            arg += ".probe";
        }

        std::ifstream stream(arg);
        std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        Config::Config* config = new Config::Config(Config::Normal, probe);

        config->modules = indexModules(arg);
        
        ProgramType* program = parser.produceAST(file);
        Val result = eval(program, env, config);

        delete program;
    } else {
        REPL* repl = new REPL();

        repl->start();

        delete repl;
    };

    for (auto& thread : getThreads()) {
        if (thread.joinable()) thread.join();
    }

    return 0;
}