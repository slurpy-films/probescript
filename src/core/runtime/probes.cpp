#include "runtime/interpreter.hpp"

Val evalProbeDeclaration(ProbeDeclarationType* probe, EnvPtr env) {
    std::shared_ptr<ProbeValue> probeval = probe->doesExtend ? std::make_shared<ProbeValue>(probe->name, env, probe->body, probe->extends) : std::make_shared<ProbeValue>(probe->name, env, probe->body);

    return env->declareVar(probe->name, probeval);
}

Val evalProbeCall(std::string probeName, EnvPtr declarationEnv, std::vector<Val> args) {
    Val val = declarationEnv->lookupVar(probeName);

    if (val->type != ValueType::Probe)
    {
        return declarationEnv->throwErr(TypeError("Probe " + probeName + " is not of type probe"));
    }

    std::shared_ptr<ProbeValue> probe = std::static_pointer_cast<ProbeValue>(val);

    EnvPtr env = std::make_shared<Env>(declarationEnv);

    inheritProbe(probe, env);

    for (Stmt* stmt : probe->body) {
        switch (stmt->kind) {
            case NodeType::AssignmentExpr: {
                AssignmentExprType* assign = static_cast<AssignmentExprType*>(stmt);
                if (assign->op != "=") {
                    std::cerr << ManualError("Only = assignment is allowed in probe bodies", "ProbeBodyError");
                    exit(1);
                }
                if (assign->assigne->kind != NodeType::Identifier) {
                    std::cerr << ManualError("Only identifiers can be assigned to in probe bodies", "ProbeBodyError");
                    exit(1);
                }
                env->variables[static_cast<IdentifierType*>(assign->assigne)->symbol] = eval(assign->value, env);
                break;
            }
            case NodeType::FunctionDeclaration:
                eval(stmt, env);
                break;
            default:
                eval(stmt, env);
        }
    }


    Val runfnval = env->lookupVar("run");

    if (runfnval->type != ValueType::Function) {
        std::cerr << "Expected run to be of type function";
        exit(1);
    }

    evalCallWithFnVal(runfnval, args, env);

    return std::make_shared<UndefinedVal>();
}

void inheritProbe(std::shared_ptr<ProbeValue> prb, EnvPtr env)
{
    if (!prb->doesExtend) return;
    
    Val extends = eval(prb->extends, prb->declarationEnv);
    EnvPtr parentenv = std::make_shared<Env>();
    
    if (extends->type != ValueType::Probe)
    {
        if (extends->type == ValueType::NativeClass)
        {
            Val instance = std::static_pointer_cast<NativeClassVal>(extends)->constructor({}, env);
            for (auto& prop : instance->properties)
            {
                env->variables[prop.first] = prop.second;
            }

            return;
        } else
        {
            std::cerr << ManualError("Probes can only inherit from probes", "ProbeInheritanceError");
            exit(1);
        }
    } else
        parentenv = std::static_pointer_cast<ProbeValue>(extends)->declarationEnv;

    std::shared_ptr<ProbeValue> superProbe = std::static_pointer_cast<ProbeValue>(extends);
    
    inheritProbe(superProbe, env);

    bool hasRun = false;
    Val run = std::make_shared<UndefinedVal>();
    for (Stmt* stmt : superProbe->body)
    {
        if (stmt->kind == NodeType::FunctionDeclaration)
        {
            std::shared_ptr<FunctionValue> fn = std::static_pointer_cast<FunctionValue>(evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(stmt), parentenv, true));
            if (fn->name == "run")
            {
                hasRun = true;
                run = fn;
            } else env->variables[fn->name] = fn;
        } else if (stmt->kind == NodeType::VarDeclaration) {
            eval(stmt, parentenv);
            eval(stmt, env);
        } else
            eval(stmt, env);
    }

    if (hasRun) env->variables["super"] = run;
}