#pragma once

#include "../runner.h"

struct MountNamespace {
    void apply(Runner& runner);
};
