#pragma once

#include "../runner.h"
#include "../util/config.h"

struct MountNamespace {
    const YamlConfig& config;
    const bool mount_root = config.config["mount_root"].as<bool>();
    const bool pid_ns = config.config["pid_namespace"].as<bool>();
    void apply(Runner& runner);
};
