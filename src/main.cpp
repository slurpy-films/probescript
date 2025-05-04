#include "parser.hpp"
#include "runtime/interpreter.hpp"
#include "stdlib/frame/frame.hpp"
#include "REPL.hpp"
#include "modules.hpp"
#include "config.hpp"
#include <iostream>
#include <string>
#include <fstream>



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
        RuntimeVal* result = eval(program, env, config);

        delete program;
        delete result;
    } else {
        REPL* repl = new REPL();

        repl->start();

        delete repl;
    };

    return 0;
}