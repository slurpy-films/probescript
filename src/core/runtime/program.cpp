#include "runtime/interpreter.hpp"

Val evalProgram(ProgramType* program, EnvPtr env, std::shared_ptr<Context> config) {
    if (config->type == RuntimeType::Normal) {
        EnvPtr scope = std::make_shared<Env>(env);
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
                        eval(stmt, scope, config);
                        break;
                    default:
                        throw std::runtime_error(ManualError("Only variable, function, class, and probe declarations are allowed in program bodies", "ProgramError"));
                }
            }
        }

        if (!foundProbe) {
            throw std::runtime_error(ManualError("Probe " + config->probeName + " is not defined", "MainError"));
        }


        std::shared_ptr<ProbeValue> probe = std::static_pointer_cast<ProbeValue>(evalProbeDeclaration(probeDeclaration, scope));

        Val lastEval = evalProbeCall(probe->name, scope);

        return lastEval;
    } else if (config->type == RuntimeType::REPL) {
        Val lastEval = std::make_shared<UndefinedVal>();
        for (Stmt* stmt : program->body) {
            lastEval = eval(stmt, env, config);
        }

        return lastEval;
    } else if (config->type == RuntimeType::Exports) {
        std::unordered_map<std::string, Val> exports;

        EnvPtr exportenv = std::make_shared<Env>();

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
                        lasteval = exporting = eval(ident, exportenv);
                        found = true;
                        
                        break;
                    }

                    case NodeType::AssignmentExpr: {
                        AssignmentExprType* a = static_cast<AssignmentExprType*>(exportstmt->exporting);
                        if (a->assigne->kind != NodeType::Identifier) {
                            std::cerr << ManualError("Cannot export non identifier assignment", "ExportError");
                        }
                        exportname = static_cast<IdentifierType*>(a->assigne)->symbol;
                        lasteval = exporting = eval(a->value, exportenv);
                        found = true;

                        break;
                    }

                    case NodeType::ProbeDeclaration: {
                        ProbeDeclarationType* probe = static_cast<ProbeDeclarationType*>(exportstmt->exporting);
                        exportname = probe->name;
                        lasteval = exporting = eval(probe, exportenv);

                        found = true;

                        break;
                    }

                    case NodeType::FunctionDeclaration: {
                        FunctionDeclarationType* fn = static_cast<FunctionDeclarationType*>(exportstmt->exporting);
                        exportname = fn->name;
                        found = true;
                        lasteval = exporting = eval(fn, exportenv);

                        break;
                    }

                    case NodeType::ClassDefinition: {
                        ClassDefinitionType* cls = static_cast<ClassDefinitionType*>(exportstmt->exporting);
                        exportname = cls->name;
                        found = true;
                        lasteval = exporting = eval(cls, exportenv);

                        break;
                    }

                    default:
                        throw std::runtime_error(ManualError("Unknown export type", "ExportError"));
                }

                exports[exportname] = exporting;
            } else {
                eval(stmt, exportenv, config);
            }
        }

        lasteval->exports = exports;

        return lasteval;
    }

    return std::make_shared<UndefinedVal>();
}