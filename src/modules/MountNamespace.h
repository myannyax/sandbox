#pragma once

#include "../runner.h"
#include "../util/config.h"

struct MountNamespace {
    const YamlConfig& config;
    void apply(Runner& runner);
};
