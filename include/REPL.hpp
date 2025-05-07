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
            std::cout << "REPL v1.0\n";
            Env* env = new Env();
            Config::Config* config = new Config::Config(Config::REPL);
            while (true) {
                std::cout << "> ";
                Parser parser;
                std::string src;
                std::getline(std::cin, src);

                if (src.find("exit") == 0) break;

                ProgramType* program = parser.produceAST(src);

                Val result = eval(program, env, config);

                std::cout << result->toString() << "\n";
            }

        }
};