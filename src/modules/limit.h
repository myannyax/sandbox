#pragma once

#include "../runner.h"
#include "../util/config.h"
#include "ptrace.h"

struct LimitsModule {
    const YamlConfig& config;
    PtraceModule& ptraceModule;
    pid_t origPid = -1;

    void apply(Runner& runner);
};
