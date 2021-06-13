#include "limit.h"
#include "../util/units.h"
#include "../util/log.h"

#include <csignal>
#include <sys/resource.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <syscall.h>

void LimitsModule::apply(Runner &runner) {
    runner.addHook(Hook::ParentBeforeExec, [&](pid_t pid) {
        origPid = pid;

        auto set_limit = [&](const std::string& limit_name, __rlimit_resource resource,
                             rlim_t soft_limit, rlim_t hard_limit) {
            struct rlimit old_limit{};
            struct rlimit new_limit{};
            if (prlimit(pid, resource, nullptr, &old_limit) != 0) {
                throw std::runtime_error{
                    "Failed to get " + limit_name + " limit of process " + std::to_string(pid)
                    + ": " + std::strerror(errno)
                };
            }
            if (old_limit.rlim_max >= hard_limit) {
                new_limit.rlim_cur = soft_limit;
                new_limit.rlim_max = hard_limit;
                if (prlimit(pid, resource, &new_limit, nullptr) != 0) {
                    throw std::runtime_error{
                        "Failed to set " + limit_name + " limit of process " + std::to_string(pid)
                        + " to " + std::to_string(soft_limit) + ": " + std::strerror(errno)
                    };
                }
            }
        };

        auto max_stack = parse_memory_size(config.get<std::string>("max_stack", "0"));
        if (max_stack > 0) {
            set_limit("stack", RLIMIT_STACK, max_stack, max_stack);
        }

        auto max_data = parse_memory_size(config.get<std::string>("max_data", "0"));
        if (max_data > 0) {
            set_limit("data", RLIMIT_DATA, max_data, max_data);
        }

        auto max_memory = parse_memory_size(config.get<std::string>("max_memory", "0"));
        if (max_memory > 0) {
            set_limit("memory", RLIMIT_AS, max_memory, max_memory);
        }

        auto max_cpu_time = parse_time(config.get<std::string>("max_cpu_time", "0"));
        if (max_cpu_time > 0) {
            set_limit("CPU time", RLIMIT_CPU, max_cpu_time, max_cpu_time + 1);
        }
    });

    ptraceModule.onSyscall(SYS_setrlimit, [&](ProcessState& state) {
        if (state.syscall.args[1] == RLIMIT_CPU) {
            state.syscall.result = -EPERM;
        }
    });
    ptraceModule.onSyscall(SYS_prlimit64, [&](ProcessState& state) {
        if (state.syscall.args[1] == RLIMIT_CPU) {
            state.syscall.result = -EPERM;
        }
    });

    ptraceModule.onStop(SIGXCPU, [](ProcessState& state) {
        MultiprocessLog::log_fatal("Terminate program: CPU time limit exceeded");
        state.quit = true;
    });

    auto max_time = parse_time(config.get<std::string>("max_time", "0"));
    if (max_time > 0) {
        runner.addHook(Hook::ParentBeforeExec, [max_time](pid_t) {
            alarm(max_time);
        });
    }
}
