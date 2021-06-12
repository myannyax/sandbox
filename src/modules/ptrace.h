#pragma once

#include "../runner.h"

struct ProcessState {
    ProcessState(pid_t pid) : pid(pid) {}
    pid_t pid;

    long syscallErr = 0;
    bool isAttached = false;
    bool quit = false;
    bool inSyscall = false;
};

struct PtraceModule {
public:
    int exitCode;

    void apply(Runner& runner);

private:
    void onTrap(ProcessState& state);
};
