#include "runtime/interpreter.hpp"

Val evalProbeDeclaration(ProbeDeclarationType* probe, Env* env) {
    std::shared_ptr<ProbeValue> probeval = probe->doesExtend ? std::make_shared<ProbeValue>(probe->name, env, probe->body, probe->extends) : std::make_shared<ProbeValue>(probe->name, env, probe->body);

    return env->declareVar(probe->name, probeval);
}

Val evalProbeCall(std::string probeName, Env* declarationEnv, std::vector<Val> args) {
    Val val = declarationEnv->lookupVar(probeName);

    if (val->type != ValueType::Probe) {
        return declarationEnv->throwErr(TypeError("Probe " + probeName + " is not of type probe"));
    }

    std::shared_ptr<ProbeValue> probe = std::static_pointer_cast<ProbeValue>(val);

    Env* env = new Env(declarationEnv);

    for (Stmt* stmt : probe->body) {
        switch (stmt->kind) {
            case NodeType::VarDeclaration:
                eval(stmt, env);
                break;
            case NodeType::FunctionDeclaration:
                eval(stmt, env);
                break;
            default:
                return env->throwErr(ManualError("Only variable and function declarations are allowed within probe bodies", "ProbeBodyError"));
        }
    }


    Val runfnval = env->lookupVar("run");

    if (runfnval->type != ValueType::Function) {
        std::cerr << "Expected run to be of type function, got " << runfnval->type;
        exit(1);
    }


    evalCallWithFnVal(runfnval, args, env);

    return std::make_shared<UndefinedVal>();
}