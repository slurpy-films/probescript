#include "runtime/interpreter.hpp"

using namespace Probescript;
using namespace Probescript::Interpreter;

Values::Val Interpreter::evalProgram(std::shared_ptr<AST::ProgramType> program, EnvPtr env, std::shared_ptr<Context> config) {
    if (config->type == RuntimeType::Normal) {
        EnvPtr scope = std::make_shared<Env>(env);
        std::shared_ptr<AST::ProbeDeclarationType> probeDeclaration;
        bool foundProbe = false;
        for (std::shared_ptr<AST::Stmt> stmt : program->body) {
            if (
                stmt->kind == AST::NodeType::ProbeDeclaration
                && std::static_pointer_cast<AST::ProbeDeclarationType>(stmt)->name == config->probeName
            ) {
                probeDeclaration = std::static_pointer_cast<AST::ProbeDeclarationType>(stmt);
                foundProbe = true;
                break;
            } else {
                switch (stmt->kind) {
                    case AST::NodeType::VarDeclaration:
                        eval(stmt, scope);
                        break;
                    case AST::NodeType::FunctionDeclaration:
                        eval(stmt, scope);
                        break;
                    case AST::NodeType::ClassDefinition:
                        eval(stmt, scope);
                        break;
                    case AST::NodeType::ProbeDeclaration:
                        eval(stmt, scope);
                        break;
                    case AST::NodeType::ImportStmt:
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


        std::shared_ptr<Values::ProbeValue> probe = std::static_pointer_cast<Values::ProbeValue>(evalProbeDeclaration(probeDeclaration, scope));

        Values::Val lastEval = evalProbeCall(probe, scope);

        return lastEval;
    } else if (config->type == RuntimeType::REPL) {
        Values::Val lastEval = std::make_shared<Values::UndefinedVal>();
        for (std::shared_ptr<AST::Stmt> stmt : program->body) {
            lastEval = eval(stmt, env, config);
        }

        return lastEval;
    } else if (config->type == RuntimeType::Exports) {
        std::unordered_map<std::string, Values::Val> exports;

        EnvPtr exportenv = std::make_shared<Env>();

        Values::Val lasteval;

        for (std::shared_ptr<AST::Stmt> stmt : program->body) {
            if (stmt->kind == AST::NodeType::ExportStmt) {
                std::shared_ptr<AST::ExportStmtType> exportstmt = std::static_pointer_cast<AST::ExportStmtType>(stmt);
                
                std::string exportname;
                Values::Val exporting;
                bool found = false;

                switch (exportstmt->exporting->kind) {
                    case AST::NodeType::Identifier: {
                        std::shared_ptr<AST::IdentifierType> ident = std::static_pointer_cast<AST::IdentifierType>(exportstmt->exporting);
                        exportname = ident->symbol;
                        lasteval = exporting = eval(ident, exportenv);
                        found = true;
                        
                        break;
                    }

                    case AST::NodeType::AssignmentExpr: {
                        std::shared_ptr<AST::AssignmentExprType> a = std::static_pointer_cast<AST::AssignmentExprType>(exportstmt->exporting);
                        if (a->assigne->kind != AST::NodeType::Identifier) {
                            std::cerr << CustomError("Cannot export non identifier assignment", "ExportError");
                        }
                        exportname = std::static_pointer_cast<AST::IdentifierType>(a->assigne)->symbol;
                        lasteval = exporting = eval(a->value, exportenv);
                        found = true;

                        break;
                    }

                    case AST::NodeType::ProbeDeclaration: {
                        std::shared_ptr<AST::ProbeDeclarationType> probe = std::static_pointer_cast<AST::ProbeDeclarationType>(exportstmt->exporting);
                        exportname = probe->name;
                        lasteval = exporting = eval(probe, exportenv);

                        found = true;

                        break;
                    }

                    case AST::NodeType::FunctionDeclaration: {
                        std::shared_ptr<AST::FunctionDeclarationType> fn = std::static_pointer_cast<AST::FunctionDeclarationType>(exportstmt->exporting);
                        exportname = fn->name;
                        found = true;
                        lasteval = exporting = eval(fn, exportenv);

                        break;
                    }

                    case AST::NodeType::ClassDefinition: {
                        std::shared_ptr<AST::ClassDefinitionType> cls = std::static_pointer_cast<AST::ClassDefinitionType>(exportstmt->exporting);
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

    return std::make_shared<Values::UndefinedVal>();
}