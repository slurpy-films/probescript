#include "parser.hpp"
#include "runtime/interpreter.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;


int main(int argc, char* argv[]) {
    Parser parser;
    Env* env = new Env();
    if (argc > 1) {
        string arg = argv[1];
        if (arg.find(".probe") == std::string::npos) {
            arg += ".probe";
        }
        
        ifstream stream(arg);
        string file((istreambuf_iterator<char>(stream)), istreambuf_iterator<char>());
        
        ProgramType* program = parser.produceAST(file);
        RuntimeVal* result = eval(program, env);
    } else {
    cout << "REPL v0.1" << endl;

    while (true) {
        string input;
        cout << "> ";
        getline(cin, input);

        if (input.find("exit") == 0) break;

        ProgramType* program = parser.produceAST(input);

        RuntimeVal* result = eval(program, env);

        cout << result->value << endl;
        
        delete program;
    }
    }

    return 0;
}