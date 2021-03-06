#include "runner.h"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

static const int stackSize = 1024 * 1024;

void Runner::run() {
    try {
        runImpl();
    } catch (...) {
        cleanup();
        throw;
    }
    cleanup();
}

void Runner::addHook(Hook hook, std::function<void(pid_t)> func) {
    hooks.emplace_back(hook, std::move(func));
}

void Runner::runImpl() {
    if (pipe2(errorReportPipe, O_CLOEXEC) == -1) {
        throw std::runtime_error{"pipe2() failed"};
    }

    std::stringstream ss;
    ss << "/sandbox_sync_" << getpid();
    syncName = ss.str();
    sync = sem_open(syncName.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);

    char* stack = (char*)malloc(stackSize);

    pid = clone([](void* arg) {
        ((Runner*)arg)->child();
        return 0;
    }, stack + stackSize, unshareFlags, this);

    if (pid < 0) {
        throw std::runtime_error{"clone() failed"};
    }

    parent();
}

void Runner::cleanup() {
    if (pid > 0) {
        kill(pid, SIGKILL);
        pid = -1;
    }

    for (int i = 0; i < 2; ++i) {
        if (errorReportPipe[i] != -1) close(errorReportPipe[i]);
        errorReportPipe[i] = -1;
    }

    if (sync) {
        sem_close(sync);
        sync = nullptr;
        sem_unlink(syncName.c_str());
    }

    callHook(Hook::Cleanup);
}

void Runner::callHook(Hook hook) const {
    for (const auto& [h, f] : hooks) {
        if (h == hook) f(pid);
    }
}

void Runner::child() noexcept {
    close(errorReportPipe[0]);
    sync = sem_open(syncName.c_str(), O_RDWR);
    sem_wait(sync); // wait for signal from parent
    sem_close(sync);
    sem_unlink(syncName.c_str());
    std::string errorMessage;

    try {
        callHook(Hook::BeforeExec);
        callHook(Hook::Exec);
    } catch (const std::exception& e) {
        errorMessage = e.what();
    }

    write(errorReportPipe[1], errorMessage.data(), errorMessage.size());
    exit(0);
}

void Runner::parent() {
    close(errorReportPipe[1]);
    callHook(Hook::ParentBeforeExec);
    sem_post(sync);

    char buf[4096];
    buf[0] = '\0';
    int z = read(errorReportPipe[0], buf, sizeof buf);
    if (buf[0]) {
        // an error occured while starting the program
        throw std::runtime_error{std::string(buf, z)};
    }
    close(errorReportPipe[0]);

    callHook(Hook::ParentAfterExec);
}

void Runner::setUnshareFlags(unsigned long flags) {
    unshareFlags |= flags;
}
