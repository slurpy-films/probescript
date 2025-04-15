#include "parser.cpp"
#include "runtime/interpreter.cpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

string nodeTypeToString(NodeType type) {
    switch (type) {
        case Program: return "Program";
        case NumericLiteral: return "NumericLiteral";
        case Identifier: return "Identifier";
        case BinaryExpr: return "BinaryExpr";
        case NullLiteral: return "NullLiteral";
        default: return "Unknown";
    }
}


void printProgram(const ProgramType* program) {
    for (const Stmt* stmt : program->body) {

        if (stmt->kind == NodeType::NumericLiteral) {
            const NumericLiteralType* num = static_cast<const NumericLiteralType*>(stmt);
            cout << "Number: " << num->numValue << endl;
        } else if (stmt->kind == NodeType::BinaryExpr) {
            const BinaryExprType* bin = static_cast<const BinaryExprType*>(stmt);
            cout << "BinaryExpr: " << bin->value() << endl;
        } else {
            cout << nodeTypeToString(stmt->kind) << endl;
        }
    }
}


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