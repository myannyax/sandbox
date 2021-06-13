#include "ptrace.h"
#include "util/config.h"


struct ForkModule {
    PtraceModule& ptraceModule;
    int fork_limit = config.get<int>("fork_limit", 10);
    const YamlConfig& config;
    void apply();
};
