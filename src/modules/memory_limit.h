#pragma once

#include "../runner.h"
#include "../util/config.h"

struct MemoryLimitsModule {
    const YamlConfig& config;

    void apply(Runner& runner);
};
