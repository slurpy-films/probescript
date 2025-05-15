#include "runtime/interpreter.hpp"

Val evalProgram(ProgramType* program, Env* env, Config::Config* config) {
    if (config->type == Config::Normal) {
        Env* scope = new Env(env);
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
            } else {
                switch (stmt->kind) {
                    case NodeType::VarDeclaration:
                        eval(stmt, scope);
                        break;
                    case NodeType::FunctionDeclaration:
                        eval(stmt, scope);
                        break;
                    case NodeType::ClassDefinition:
                        eval(stmt, scope);
                        break;
                    case NodeType::ProbeDeclaration:
                        eval(stmt, scope);
                        break;
                    case NodeType::ImportStmt:
                        eval(stmt, scope);
                        break;
                    default:
                        std::cerr << "Only variable, function, class, and probe declarations are allowed in program bodies, got " << static_cast<IdentifierType*>(stmt)->symbol;
                        exit(1);
                }
            }
        }

        if (!foundProbe) {
            std::cerr << "Probe " << config->probeName << " is not declared";
            exit(1);
        }


        std::shared_ptr<ProbeValue> probe = std::static_pointer_cast<ProbeValue>(evalProbeDeclaration(probeDeclaration, scope));

        Val lastEval = evalProbeCall(probe->name, scope);

        return lastEval;
    } else if (config->type == Config::REPL) {
        Val lastEval = std::make_shared<UndefinedVal>();
        for (Stmt* stmt : program->body) {
            lastEval = eval(stmt, env, config);
        }

        return lastEval;
    } else if (config->type == Config::Exports) {
        std::unordered_map<std::string, Val> exports;

        Env* env = new Env();

        Val lasteval;

        for (Stmt* stmt : program->body) {
            if (stmt->kind == NodeType::ExportStmt) {
                ExportStmtType* exportstmt = static_cast<ExportStmtType*>(stmt);
                
                std::string exportname;
                Val exporting;
                bool found = false;

                switch (exportstmt->exporting->kind) {
                    case NodeType::Identifier: {
                        IdentifierType* ident = static_cast<IdentifierType*>(exportstmt->exporting);
                        exportname = ident->symbol;
                        lasteval = exporting = eval(ident, env);
                        found = true;
                        
                        break;
                    }

                    case NodeType::ProbeDeclaration: {
                        ProbeDeclarationType* probe = static_cast<ProbeDeclarationType*>(exportstmt->exporting);
                        exportname = probe->name;
                        lasteval = exporting = eval(probe, env);
                        found = true;

                        break;
                    }

                    case NodeType::FunctionDeclaration: {
                        FunctionDeclarationType* fn = static_cast<FunctionDeclarationType*>(exportstmt->exporting);
                        exportname = fn->name;
                        found = true;
                        lasteval = exporting = eval(fn, env);

                        break;
                    }

                    case NodeType::ClassDefinition: {
                        ClassDefinitionType* cls = static_cast<ClassDefinitionType*>(exportstmt->exporting);
                        exportname = cls->name;
                        found = true;
                        lasteval = exporting = eval(cls, env);

                        break;
                    }

                    default:
                        std::cerr << "Type " << exportstmt->exporting->kind << " cannot be exported";
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

    return std::make_shared<UndefinedVal>();
}