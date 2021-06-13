#pragma once

#include "runner.h"
#include "../util/config.h"

struct UserNamespace {
    const YamlConfig& config;
    void apply(Runner& runner);
};
