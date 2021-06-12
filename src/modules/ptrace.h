#pragma once

#include "../runner.h"
#include <array>

struct Syscall {
    long nr;
    std::array<long, 6> args;
    long result;
};

struct ProcessState {
    ProcessState(pid_t pid) : pid(pid) {}
    pid_t pid;

    bool isAttached = false;
    bool quit = false;
    bool inSyscall = false;
    Syscall syscall;

    std::string readString(void* addr, size_t maxSize = 65536) const;
};

struct PtraceModule {
public:
    int exitCode;

    void apply(Runner& runner);
    std::function<void(ProcessState&)> onSyscall;

private:
    void onTrap(ProcessState& state);
};
