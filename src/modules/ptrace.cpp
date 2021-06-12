#include "ptrace.h"

#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <memory.h>

static void restrictProcess(pid_t pid) {
    ptrace(PTRACE_SETOPTIONS, pid, nullptr,
            PTRACE_O_EXITKILL // when the tracer exits, kill the tracee
          | PTRACE_O_TRACECLONE // monitor clone()d processes
          | PTRACE_O_TRACEFORK // monitor fork()ed processes
          | PTRACE_O_TRACEVFORK // monitor vfork()ed processes
    );
}

std::string ProcessState::readString(void* addr, size_t maxSize) const {
    char *ptr = (char*)addr;
    std::string res;
    if (!ptr) {
        return res;
    }

    while (res.size() < maxSize) {
        long word = ptrace(PTRACE_PEEKDATA, pid, ptr, NULL);
        if (word == -1) {
            break;
        }
        ptr += sizeof(long);
        char *c = (char*)&word;
        for (size_t n = 0; n < sizeof(word); ++c, ++n) {
            if (*c) {
                res.push_back(*c);
            } else {
                return res;
            }
        }
    }

    return res;
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
                state.isAttached = true;
                restrictProcess(pid);
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
                for (auto& [signal, handler] : signalHandlers) {
                    if (signal == sig) {
                        handler(state);
                    }
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

void PtraceModule::onSyscall(long syscall, std::function<void(ProcessState&)> handler) {
    syscallHandlers.emplace_back(syscall, std::move(handler));
}

void PtraceModule::onStop(long signal, std::function<void(ProcessState&)> handler) {
    signalHandlers.emplace_back(signal, std::move(handler));
}

void PtraceModule::onTrap(ProcessState& state) {
    state.inSyscall = !state.inSyscall;
    if (!state.inSyscall) {
        if (state.syscall.result) {
            ptrace(PTRACE_POKEUSER, state.pid, sizeof(long) * RAX, (void*)(state.syscall.result));
            state.syscall.result = 0;
        }
        return;
    }

    state.syscall.nr      = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * ORIG_RAX, nullptr);
    state.syscall.args[0] = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * RDI     , nullptr);
    state.syscall.args[1] = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * RSI     , nullptr);
    state.syscall.args[2] = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * RDX     , nullptr);
    state.syscall.args[3] = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * R10     , nullptr);
    state.syscall.args[4] = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * R8      , nullptr);
    state.syscall.args[5] = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * R9      , nullptr);
    state.syscall.result  = 0;

    for (auto& [syscall, handler] : syscallHandlers) {
        if (syscall == state.syscall.nr) {
            handler(state);
        }
    }
    if (state.syscall.result) {
        ptrace(PTRACE_POKEUSER, state.pid, sizeof(long) * ORIG_RAX, (void*)-1);
    }
}
