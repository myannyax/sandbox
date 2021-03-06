#include "ptrace.h"
#include "util/log.h"
#include "util/syscall_numbers.h"

#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <memory.h>
#include <cstring>

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

static std::unordered_map<pid_t, ProcessState> states;
static bool isPaused = false;

static void pauseUnpause() {
    isPaused = !isPaused;

    for (auto& [pid, _] : states) {
        if (isPaused) {
            kill(pid, SIGSTOP);
        } else {
            ptrace(PTRACE_SYSCALL, pid, NULL, 0);
        }
    }
}

static void stopSandbox() {
    MultiprocessLog::log_info("Terminating process because of user's request");
    for (auto& [pid, _] : states) {
        kill(pid, SIGKILL);
    }
}

void PtraceModule::apply(Runner& runner, const YamlConfig& config) {
    if (config.get<bool>("pid_namespace", false)) {
        onSyscall(SYS_kill, [&](ProcessState& state) {
            int sig = state.syscall.args[1];
            if (sig == SIGTRAP) {
                state.syscall.result = -EPERM;
            }
        });
    } else {
        onSyscall(SYS_kill, [&](ProcessState& state) {
            pid_t pid = state.syscall.args[0];
            int sig = state.syscall.args[1];
            if (sig == SIGTRAP || !states.contains(pid)) {
                state.syscall.result = -EPERM;
            }
        });
    }

    runner.addHook(Hook::BeforeExec, [](pid_t) {
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    });

    runner.addHook(Hook::ParentBeforeExec, [](pid_t) {
        signal(SIGTSTP, [](int) {
            pauseUnpause();
        });

        signal(SIGQUIT, [](int) {
            stopSandbox();
        });

        signal(SIGALRM, [](int) {
            MultiprocessLog::log_fatal("Terminate program: time limit exceeded");
            for (auto& [pid, _] : states) {
                kill(pid, SIGKILL);
            }
        });
    });

    runner.addHook(Hook::ParentAfterExec, [this](pid_t origPid) {
        states.clear();
        states.emplace(origPid, origPid);

        exitCode = -1;

        while (!states.empty()) {
            int status;
            auto pid = wait(&status);
            if (pid == -1) break;
            if (!states.contains(pid)) states.emplace(pid, pid);
            auto& state = states.at(pid);

            if (!state.isAttached && WIFSTOPPED(status) && (WSTOPSIG(status) == SIGTRAP || WSTOPSIG(status) == SIGSTOP)) {
                state.isAttached = true;
                restrictProcess(pid);
                ptrace(PTRACE_SYSCALL, pid, NULL, 0);
                continue;
            }

            if (WIFEXITED(status)) {
                if (pid == origPid) {
                    exitCode = WEXITSTATUS(status);
                    MultiprocessLog::log_info("Exit code: " + std::to_string(exitCode));
                }

                states.erase(pid);
                continue;
            }

            if (WIFSIGNALED(status)) {
                if (pid == origPid) {
                    exitCode = -WTERMSIG(status);
                    MultiprocessLog::log_info(
                        "Process signaled: " + std::string(strsignal(WTERMSIG(status)))
                    );
                }

                states.erase(pid);
                continue;
            }

            if (WIFSTOPPED(status)) {
                int sig = WSTOPSIG(status);
                if (sig == SIGTRAP) {
                    // syscall
                    sig = 0;
                    onTrap(state);
                } else if (sig == SIGQUIT) {
                    sig = 0;
                    stopSandbox();
                }

                for (auto& [signal, handler] : signalHandlers) {
                    if (signal == sig) {
                        handler(state);
                    }
                }

                if (!state.quit && !isPaused) {
                    ptrace(PTRACE_SYSCALL, pid, NULL, sig);
                }
            }

            if (state.quit) {
                if (state.pid == origPid) {
                    MultiprocessLog::log_info("Process signaled: Killed");
                }
                kill(pid, SIGKILL);
                states.erase(pid);
                break;
            }
        }

        for (auto& [pid, _] : states) {
            kill(pid, SIGKILL);
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
        long retval = ptrace(PTRACE_PEEKUSER, state.pid, sizeof(long) * RAX, nullptr);
        if (retval == -EPERM || retval == -EACCES) {
            if (state.syscall.nr >= 0 && state.syscall.nr < (int)syscallNumbers().size()) {
                MultiprocessLog::log_info("Permission denied while trying to perform system call " + syscallNumbers()[state.syscall.nr]);
            }
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
    } else {
        if (state.syscall.nr == SYS_execve) {
            state.inSyscall = false;
        }
    }
}
