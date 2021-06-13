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
    Cleanup
};

class Runner {
public:
    void run();
    void addHook(Hook, std::function<void(pid_t)>);
    void setUnshareFlags(unsigned long flags);

private:
    void parent();
    void child() noexcept;

    void callHook(Hook hook) const;
    void cleanup();
    void runImpl();

private:
    pid_t pid;
    unsigned long unshareFlags = 0;
    int errorReportPipe[2];
    sem_t* sync; // for synchronizing start of the child process
    std::string syncName;

    std::vector<std::pair<Hook, std::function<void(pid_t)>>> hooks;
};
