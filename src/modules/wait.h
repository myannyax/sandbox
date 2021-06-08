#pragma once

#include "../runner.h"

struct WaitModule {
    int exitCode;

    void apply(Runner& runner);
};
