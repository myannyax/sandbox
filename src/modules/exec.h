#pragma once

#include "../runner.h"

#include <vector>
#include <string>

struct ExecModule {
    std::string exe;
    std::vector<std::string> argv, env;
    bool inheritEnv;

    void apply(Runner& runner);
};
