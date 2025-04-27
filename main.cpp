#include "parser.hpp"
#include "runtime/interpreter.hpp"
#include "REPL.hpp"
#include "modules.hpp"
#include "config.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;


int main(int argc, char* argv[]) {
    Parser parser;
    Env* env = new Env();
    if (argc > 1) {
        string probe = "Main";
        string arg;
        for (size_t i = 0; i < argc; ++i) {
            if (argv[i] == "-P") {
                probe = argv[i + 1];
            } else arg = argv[i];
        }
        if (arg.find(".probe") == std::string::npos) {
            arg += ".probe";
        }

        ifstream stream(arg);
        string file((istreambuf_iterator<char>(stream)), istreambuf_iterator<char>());

        Config::Config* config = new Config::Config(Config::Normal, probe);

        config->modules = indexModules();
        
        ProgramType* program = parser.produceAST(file);
        RuntimeVal* result = eval(program, env, config);
    } else {
        REPL* repl = new REPL();

        repl->start();

        delete repl;
    };

    return 0;
}