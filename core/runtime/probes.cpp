#include "runtime/interpreter.hpp"

using namespace Probescript;
using namespace Probescript::Interpreter;

Values::Val Interpreter::evalProbeDeclaration(std::shared_ptr<AST::ProbeDeclarationType> probe, EnvPtr env) {
    std::shared_ptr<Values::ProbeValue> probeval = probe->doesExtend ? std::make_shared<Values::ProbeValue>(probe->name, env, probe->body, probe->extends) : std::make_shared<Values::ProbeValue>(probe->name, env, probe->body);

    return env->declareVar(probe->name, probeval, probe->token);
}

Values::Val Interpreter::evalProbeCall(Values::Val val, EnvPtr declarationEnv, std::vector<Values::Val> args) {
    if (val->type != Values::ValueType::Probe)
    {
        throw ThrowException(TypeError("Probe is not of type probe"));
    }

    std::shared_ptr<Values::ProbeValue> probe = std::static_pointer_cast<Values::ProbeValue>(val);

    EnvPtr env = std::make_shared<Env>(declarationEnv);

    inheritProbe(probe, env);

    for (std::shared_ptr<AST::Stmt> stmt : probe->body) {
        switch (stmt->kind) {
            case AST::NodeType::AssignmentExpr: {
                std::shared_ptr<AST::AssignmentExprType> assign = std::static_pointer_cast<AST::AssignmentExprType>(stmt);
                if (assign->op != "=") {
                    throw std::runtime_error(CustomError("Only = assignment is allowed in probe bodies", "ProbeBodyError"));
                }
                if (assign->assigne->kind != AST::NodeType::Identifier) {
                    throw std::runtime_error(CustomError("Only identifiers can be assigned to in probe bodies", "ProbeBodyError"));
                }
                env->variables[std::static_pointer_cast<AST::IdentifierType>(assign->assigne)->symbol] = eval(assign->value, env);
                break;
            }
            case AST::NodeType::FunctionDeclaration:
                eval(stmt, env);
                break;
            default:
                eval(stmt, env);
        }
    }


    Values::Val runfnval = env->lookupVar("run", val->token);

    if (runfnval->type != Values::ValueType::Function) {
        throw std::runtime_error(CustomError("Expected 'run' to be of type function", "ProbeError"));
    }

    evalCallWithFnVal(runfnval, args, env);

    return std::make_shared<Values::UndefinedVal>();
}

void Interpreter::inheritProbe(std::shared_ptr<Values::ProbeValue> prb, EnvPtr env)
{
    if (!prb->doesExtend) return;
    
    Values::Val extends = eval(prb->extends, prb->declarationEnv);
    EnvPtr parentenv = std::make_shared<Env>();
    
    if (extends->type != Values::ValueType::Probe)
    {
        if (extends->type == Values::ValueType::NativeClass)
        {
            Values::Val instance = std::static_pointer_cast<Values::NativeClassVal>(extends)->constructor({}, env);
            for (auto& prop : instance->properties)
            {
                env->variables[prop.first] = prop.second;
            }

            return;
        } else
        {
            throw std::runtime_error(CustomError("Probes can only inherit from probes", "ProbeInheritanceError"));
        }
    } else
        parentenv = std::static_pointer_cast<Values::ProbeValue>(extends)->declarationEnv;

    std::shared_ptr<Values::ProbeValue> superProbe = std::static_pointer_cast<Values::ProbeValue>(extends);
    
    inheritProbe(superProbe, env);

    bool hasRun = false;
    Values::Val run = std::make_shared<Values::UndefinedVal>();
    for (std::shared_ptr<AST::Stmt> stmt : superProbe->body)
    {
        if (stmt->kind == AST::NodeType::FunctionDeclaration)
        {
            std::shared_ptr<Values::FunctionValue> fn = std::static_pointer_cast<Values::FunctionValue>(evalFunctionDeclaration(std::static_pointer_cast<AST::FunctionDeclarationType>(stmt), parentenv, true));
            if (fn->name == prb->name)
            {
                hasRun = true;
                run = fn;
            }
            else env->variables[fn->name] = fn;
        }
        else if (stmt->kind == AST::NodeType::VarDeclaration) {
            eval(stmt, parentenv);
            eval(stmt, env);
        }
        else
            eval(stmt, env);
    }

    if (hasRun) env->variables["super"] = run;
}