#include "fork.h"

void ForkModule::apply() {
    if (fork_limit == -1) return;
    ptraceModule.onSyscall(SYS_clone, [&](ProcessState& state) {
        if (fork_limit == 0) {
            state.syscall.result = -EPERM;
        } else {
            fork_limit--;
        }
    });

    ptraceModule.onSyscall(SYS_fork, [&](ProcessState& state) {
        if (fork_limit == 0) {
            state.syscall.result = -EPERM;
        } else {
            fork_limit--;
        }
    });
}
