#pragma once

#include <vector>
#include <string>
#include <functional>

#include <sys/types.h>
#include <semaphore.h>

enum class Hook {
    BeforeExec,
    Exec,
    ParentBeforeExec,
    ParentAfterExec,
};

class Runner {
public:
    void run();

private:
    void child() noexcept;
    void parent();

    void callHook(Hook hook) const;
    void cleanup();
    void runImpl();

private:
    pid_t pid;
    int errorReportPipe[2];
    sem_t sync; // for synchronizing start of the child process
    bool syncInited;

    std::vector<std::pair<Hook, std::function<void(pid_t)>>> hooks;
};
