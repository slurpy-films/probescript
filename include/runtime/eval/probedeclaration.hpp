#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

RuntimeVal* evalProbeDeclaration(ProbeDeclarationType* probe, Env* env) {
    ProbeValue* probeval = new ProbeValue(probe->name, env, probe->body);

    return env->declareVar(probe->name, probeval);
}