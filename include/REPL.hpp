#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "runtime/interpreter.hpp"
#include "parser.hpp"
#include "config.hpp"

class REPL {
    public:
        void start() {
            cout << "REPL v1.0\n";
            Env* env = new Env();
            Config::Config* config = new Config::Config(Config::REPL);
            while (true) {
                cout << "> ";
                Parser parser;
                string src;
                getline(cin, src);

                if (src.find("exit") == 0) break;

                ProgramType* program = parser.produceAST(src);

                RuntimeVal* result = eval(program, env, config);

                cout << result->value << "\n";

                delete program;
            }

        }
};