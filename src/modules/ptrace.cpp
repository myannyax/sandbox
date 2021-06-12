#include "ptrace.h"

#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>

static void restrictProcess(pid_t pid) {
    ptrace(PTRACE_SETOPTIONS, pid, nullptr,
            PTRACE_O_EXITKILL // when the tracer exits, kill the tracee
          | PTRACE_O_TRACECLONE // monitor clone()d processes
          | PTRACE_O_TRACEFORK // monitor fork()ed processes
          | PTRACE_O_TRACEVFORK // monitor vfork()ed processes
    );
}

void PtraceModule::apply(Runner& runner) {
    runner.addHook(Hook::BeforeExec, [](pid_t) {
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    });

    runner.addHook(Hook::ParentAfterExec, [this](pid_t origPid) {
        std::unordered_map<pid_t, ProcessState> states;
        states.emplace(origPid, origPid);
        bool keepGoing = true;

        while (keepGoing) {
            int status;
            auto pid = wait(&status);
            if (pid == -1) break;
            if (!states.contains(pid)) states.emplace(pid, pid);
            auto& state = states.at(pid);

            if (!state.isAttached) {
                if (WIFSTOPPED(status) && (WSTOPSIG(status) == SIGTRAP || WSTOPSIG(status) == SIGSTOP)) {
                    // attached
                    state.isAttached = true;
                    restrictProcess(pid);
                    ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr);
                    continue;
                } else {
                    // something failed
                    for (auto& [pid, _] : states) {
                        kill(pid, SIGKILL);
                    }

                    std::stringstream ss;
                    ss << "couldn't attach to pid " << pid;
                    throw std::runtime_error{ss.str()};
                }
            }

            if (WIFEXITED(status)) {
                if (pid == origPid) {
                    exitCode = WEXITSTATUS(status);
                    keepGoing = false;
                }

                states.erase(pid);
                continue;
            }

            if (WIFSIGNALED(status)) {
                if (pid == origPid) {
                    exitCode = -WTERMSIG(status);
                    keepGoing = false;
                }

                states.erase(pid);
                continue;
            }

            if (WIFSTOPPED(status)) {
                int sig = WSTOPSIG(status);
                if (((status >> 8) & 0xff) == SIGTRAP) {
                    // syscall
                    sig = 0;
                    onTrap(state);
                }

                if (!state.quit) {
                    ptrace(PTRACE_SYSCALL, pid, NULL, sig);
                }
            }

            if (state.quit) {
                kill(pid, SIGKILL);
            }
        }
    });
}

void PtraceModule::onTrap(ProcessState& state) {
    state.inSyscall = !state.inSyscall;
    if (!state.inSyscall) {
        if (state.syscallErr) {
            ptrace(PTRACE_POKEUSER, state.pid, sizeof(long) * RAX, (void*)(state.syscallErr));
            state.syscallErr = 0;
        }
        return;
    }

    long nr = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * ORIG_RAX, nullptr);
}
