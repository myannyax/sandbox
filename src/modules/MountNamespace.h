#pragma once

#include "../runner.h"
#include "../util/config.h"

struct MountNamespace {
    const YamlConfig& config;
    const bool mount_root = config.get<bool>("mount_root", false);
    const bool pid_ns = config.get<bool>("pid_namespace", false);
    void apply(Runner& runner);
};
