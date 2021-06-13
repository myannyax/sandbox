#pragma once

#include "../runner.h"
#include "../util/config.h"
#include "ptrace.h"

struct PriorityModule {
    const YamlConfig& config;
    PtraceModule& ptraceModule;

    void apply(Runner& runner);
};
