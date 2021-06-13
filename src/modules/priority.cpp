#include "priority.h"
#include "../util/log.h"

#include <sys/resource.h>
#include <cstring>

void PriorityModule::apply(Runner &runner) {
    runner.addHook(Hook::ParentBeforeExec, [&](pid_t pid) {
        auto niceness_opt = config.get<int>("niceness");
        if (!niceness_opt) {
            return;
        }
        int niceness = niceness_opt.value();
        if (setpriority(PRIO_PROCESS, pid, niceness) != 0) {
            MultiprocessLog::log_error(
                "Failed to set niceness of process " + std::to_string(pid)
                + " to " + std::to_string(niceness) + ": " + std::strerror(errno));
        }
    });
}
