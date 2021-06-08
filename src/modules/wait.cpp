#include "wait.h"

#include <sys/wait.h>

void WaitModule::apply(Runner& runner) {
    runner.addHook(Hook::ParentAfterExec, [&](pid_t pid) {
        int status;

        while (1) {
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                exitCode = WEXITSTATUS(status);
                return;
            }

            if (WIFSIGNALED(status)) {
                exitCode = -WTERMSIG(status);
                return;
            }
        }
    });
}
