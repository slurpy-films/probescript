#pragma once
#include "ast.hpp"
#include "runtime/env.hpp"
#include "runtime/values.hpp"

Val evalProbeDeclaration(ProbeDeclarationType* probe, Env* env) {
    std::shared_ptr<ProbeValue> probeval = probe->doesExtend ? std::make_shared<ProbeValue>(probe->name, env, probe->body, probe->extends) : std::make_shared<ProbeValue>(probe->name, env, probe->body);

    return env->declareVar(probe->name, probeval);
}