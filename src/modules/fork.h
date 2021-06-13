#include "ptrace.h"
#include "util/config.h"

#include <sys/syscall.h>

struct ForkModule {
    PtraceModule& ptraceModule;
    const YamlConfig& config;
    int fork_limit = config.get<int>("fork_limit", 10);
    void apply();
};
