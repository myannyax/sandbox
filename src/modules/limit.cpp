#include "limit.h"
#include "../util/units.h"

#include <cmath>
#include <csignal>
#include <sys/resource.h>
#include <iostream>

void LimitsModule::apply(Runner &runner) {
    runner.addHook(Hook::ParentBeforeExec, [&](pid_t pid) {
        auto set_limit = [&](const std::string& limit_name, __rlimit_resource resource, rlim_t limit_value) {
            struct rlimit old_limit{};
            struct rlimit new_limit{};
            if (prlimit(pid, resource, nullptr, &old_limit) != 0) {
                throw std::runtime_error{
                    "Failed to get " + limit_name + " limit of process " + std::to_string(pid)
                    + ": " + std::strerror(errno)
                };
            }
            if (old_limit.rlim_max > limit_value) {
                new_limit.rlim_cur = limit_value;
                new_limit.rlim_max = RLIM_INFINITY;
                if (prlimit(pid, resource, &new_limit, nullptr) != 0) {
                    throw std::runtime_error{
                        "Failed to set " + limit_name + " limit of process " + std::to_string(pid)
                        + " to " + std::to_string(limit_value) + ": " + std::strerror(errno)
                    };
                }
            }
        };

        auto max_stack = parse_memory_size(config.get<std::string>("max_stack", "0"));
        if (max_stack > 0) {
            set_limit("stack", RLIMIT_STACK, max_stack);
        }

        auto max_data = parse_memory_size(config.get<std::string>("max_data", "0"));
        if (max_data > 0) {
            set_limit("data", RLIMIT_DATA, max_data);
        }

        auto max_memory = parse_memory_size(config.get<std::string>("max_memory", "0"));
        if (max_memory > 0) {
            set_limit("memory", RLIMIT_AS, max_memory);
        }

        auto max_cpu_time = parse_time(config.get<std::string>("max_cpu_time", "0"));
        if (max_cpu_time > 0) {
            set_limit("CPU time", RLIMIT_CPU, std::ceil(max_cpu_time / 1000));
        }

        auto max_time = parse_time(config.get<std::string>("max_time", "0"));
        if (max_time > 0) {
            auto timer_pid = fork();
            if (timer_pid < 0) {
                throw std::runtime_error{"fork() failed"};
            }
            if (timer_pid == 0) {
                usleep(max_time * 1000);
                std::cout << "Time limit exceeded" << std::endl;
                kill(pid, SIGKILL);
                exit(0);
            } else {
                runner.addHook(Hook::ParentAfterExec, [&](pid_t) {
                   kill(timer_pid, SIGKILL);
                });
            }
        }
    });

    ptraceModule.onStop(SIGSEGV, [&](ProcessState& state) {
        std::cout << "SIGSEGV" << std::endl;
        state.quit = true;
    });

    ptraceModule.onStop(ENOMEM, [&](ProcessState& state) {
        std::cout << "Memory limit exceeded" << std::endl;
        state.quit = true;
    });

    ptraceModule.onStop(SIGXCPU, [&](ProcessState& state) {
        std::cout << "CPU time limit exceeded" << std::endl;
        state.quit = true;
    });
}
