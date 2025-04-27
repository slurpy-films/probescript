#pragma once
#include <filesystem>
#include <fstream>
#include "ast.hpp"
#include "runtime/values.hpp"
#include "runtime/env.hpp"
#include "config.hpp"
#include "runprobe.hpp"
#include "probedeclaration.hpp"
#include "runtime/interpreter.hpp"
#include "parser.hpp"

namespace fs = std::filesystem;

RuntimeVal* evalProgram(ProgramType* program, Env* env, Config::Config* config) {
    if (config->type == Config::Normal) {
        ProbeDeclarationType* probeDeclaration;
        bool foundProbe = false;
        for (Stmt* stmt : program->body) {
            if (
                stmt->kind == NodeType::ProbeDeclaration
                && static_cast<ProbeDeclarationType*>(stmt)->name == config->probeName
            ) {
                probeDeclaration = static_cast<ProbeDeclarationType*>(stmt);
                foundProbe = true;
                break;
            } else if (stmt->kind == NodeType::ImportStmt) {
                ImportStmtType* importstmt = static_cast<ImportStmtType*>(stmt);
                string modulename = importstmt->module;
                fs::path filepath = config->modules[modulename];

                ifstream stream(filepath);

                string file((istreambuf_iterator<char>(stream)), istreambuf_iterator<char>());

                Parser parser;

                ProgramType* program = parser.produceAST(file);

                RuntimeVal* evaluated = eval(program, new Env(), new Config::Config(Config::Exports));

                ObjectVal* moduleObj = new ObjectVal(evaluated->exports);

                env->declareVar(modulename, moduleObj, true);
            }
        }

        if (!foundProbe) {
            cerr << "Probe " << config->probeName << " is not declared";
            exit(1);
        }


        ProbeValue* probe = static_cast<ProbeValue*>(evalProbeDeclaration(probeDeclaration, env));

        RuntimeVal* lastEval = evalProbeCall(probe->name, env);

        return lastEval;
    } else if (config->type == Config::REPL) {
        RuntimeVal* lastEval = new UndefinedVal();
        for (Stmt* stmt : program->body) {
            lastEval = eval(stmt, env, config);
        }

        return lastEval;
    } else if (config->type == Config::Exports) {
        unordered_map<string, RuntimeVal*> exports;

        Env* env = new Env();

        RuntimeVal* lasteval;

        for (Stmt* stmt : program->body) {
            if (stmt->kind == NodeType::ExportStmt) {
                ExportStmtType* exportstmt = static_cast<ExportStmtType*>(stmt);
                
                string exportname;
                RuntimeVal* exporting;

                switch (exportstmt->exporting->kind) {
                    case NodeType::Identifier: {
                        IdentifierType* ident = static_cast<IdentifierType*>(exportstmt->exporting);
                        exportname = ident->symbol;
                        lasteval = exporting = eval(ident, env);
                        break;
                    }

                    default:
                        cerr << "Type " << exportstmt->exporting->kind << " cannot be exported";
                        exit(1);
                }

                exports[exportname] = exporting;
            } else {
                eval(stmt, env);
            }
        }

        lasteval->exports = exports;

        return lasteval;
    }

    return new UndefinedVal();
}