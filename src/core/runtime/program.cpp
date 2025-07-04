#include "runtime/interpreter.hpp"

Val evalProgram(std::shared_ptr<ProgramType> program, EnvPtr env, std::shared_ptr<Context> config) {
    if (config->type == RuntimeType::Normal) {
        EnvPtr scope = std::make_shared<Env>(env);
        std::shared_ptr<ProbeDeclarationType> probeDeclaration;
        bool foundProbe = false;
        for (std::shared_ptr<Stmt> stmt : program->body) {
            if (
                stmt->kind == NodeType::ProbeDeclaration
                && std::static_pointer_cast<ProbeDeclarationType>(stmt)->name == config->probeName
            ) {
                probeDeclaration = std::static_pointer_cast<ProbeDeclarationType>(stmt);
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
                        throw std::runtime_error(CustomError("Only variable, function, class, and probe declarations are allowed in program bodies", "ProgramError"));
                }
            }
        }

        if (!foundProbe) {
            throw std::runtime_error(CustomError("Probe " + config->probeName + " is not defined", "MainError"));
        }


        std::shared_ptr<ProbeValue> probe = std::static_pointer_cast<ProbeValue>(evalProbeDeclaration(probeDeclaration, scope));

        Val lastEval = evalProbeCall(probe, scope);

        return lastEval;
    } else if (config->type == RuntimeType::REPL) {
        Val lastEval = std::make_shared<UndefinedVal>();
        for (std::shared_ptr<Stmt> stmt : program->body) {
            lastEval = eval(stmt, env, config);
        }

        return lastEval;
    } else if (config->type == RuntimeType::Exports) {
        std::unordered_map<std::string, Val> exports;

        EnvPtr exportenv = std::make_shared<Env>();

        Val lasteval;

        for (std::shared_ptr<Stmt> stmt : program->body) {
            if (stmt->kind == NodeType::ExportStmt) {
                std::shared_ptr<ExportStmtType> exportstmt = std::static_pointer_cast<ExportStmtType>(stmt);
                
                std::string exportname;
                Val exporting;
                bool found = false;

                switch (exportstmt->exporting->kind) {
                    case NodeType::Identifier: {
                        std::shared_ptr<IdentifierType> ident = std::static_pointer_cast<IdentifierType>(exportstmt->exporting);
                        exportname = ident->symbol;
                        lasteval = exporting = eval(ident, exportenv);
                        found = true;
                        
                        break;
                    }

                    case NodeType::AssignmentExpr: {
                        std::shared_ptr<AssignmentExprType> a = std::static_pointer_cast<AssignmentExprType>(exportstmt->exporting);
                        if (a->assigne->kind != NodeType::Identifier) {
                            std::cerr << CustomError("Cannot export non identifier assignment", "ExportError");
                        }
                        exportname = std::static_pointer_cast<IdentifierType>(a->assigne)->symbol;
                        lasteval = exporting = eval(a->value, exportenv);
                        found = true;

                        break;
                    }

                    case NodeType::ProbeDeclaration: {
                        std::shared_ptr<ProbeDeclarationType> probe = std::static_pointer_cast<ProbeDeclarationType>(exportstmt->exporting);
                        exportname = probe->name;
                        lasteval = exporting = eval(probe, exportenv);

                        found = true;

                        break;
                    }

                    case NodeType::FunctionDeclaration: {
                        std::shared_ptr<FunctionDeclarationType> fn = std::static_pointer_cast<FunctionDeclarationType>(exportstmt->exporting);
                        exportname = fn->name;
                        found = true;
                        lasteval = exporting = eval(fn, exportenv);

                        break;
                    }

                    case NodeType::ClassDefinition: {
                        std::shared_ptr<ClassDefinitionType> cls = std::static_pointer_cast<ClassDefinitionType>(exportstmt->exporting);
                        exportname = cls->name;
                        found = true;
                        lasteval = exporting = eval(cls, exportenv);

                        break;
                    }

                    default:
                        throw std::runtime_error(CustomError("Unknown export type", "ExportError"));
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