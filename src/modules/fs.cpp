#include "fs.h"

#include <sys/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <unistd.h>

#include <iostream>

namespace fs = std::filesystem;

static fs::path readLink(const std::string& s) {
    char buf[PATH_MAX + 1];
    int st = readlink(s.c_str(), buf, PATH_MAX);
    if (st == -1) return {};
    buf[st] = 0;
    return buf;
}

void FilesystemModule::apply() {
    if (!config.config["fs"].IsSequence()) {
        return;
    }

    for (const auto& rule : config.config["fs"]) {
        FilesystemRule r;
        r.path = rule["path"].as<std::string>();
        auto act = rule["action"].as<std::string>();
        if (act == "allow") r.action = FilesystemAction::Allow;
        if (act == "deny") r.action = FilesystemAction::Deny;
        if (act == "readonly") r.action = FilesystemAction::Readonly;
        rules.push_back(r);
    }

    ptraceModule.onSyscall(SYS_open, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        unsigned flags = state.syscall.args[1];
        handle(state, path, flags);
    });

    ptraceModule.onSyscall(SYS_creat, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        unsigned flags = state.syscall.args[1];
        handle(state, path, flags);
    });

    ptraceModule.onSyscall(SYS_openat, [&](ProcessState& state) {
        int fd = state.syscall.args[0];
        fs::path path = state.readString((void*)state.syscall.args[1]);
        unsigned flags = state.syscall.args[2];

        handleAt(state, path, flags, fd);
    });

#ifdef SYS_openat2
    ptraceModule.onSyscall(SYS_openat2, [&](ProcessState& state) {
        state.syscall.result = -EINVAL;
    });
#endif

    ptraceModule.onSyscall(SYS_stat, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        handle(state, path, 0);
    });

    ptraceModule.onSyscall(SYS_lstat, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        handle(state, path, 0);
    });

    ptraceModule.onSyscall(SYS_newfstatat, [&](ProcessState& state) {
        int fd = state.syscall.args[0];
        fs::path path = state.readString((void*)state.syscall.args[1]);
        handleAt(state, path, 0, fd);
    });

    ptraceModule.onSyscall(SYS_mkdir, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        handle(state, path, O_WRONLY);
    });

    ptraceModule.onSyscall(SYS_rmdir, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        handle(state, path, O_WRONLY);
    });

    ptraceModule.onSyscall(SYS_mkdirat, [&](ProcessState& state) {
        int fd = state.syscall.args[0];
        fs::path path = state.readString((void*)state.syscall.args[1]);
        handleAt(state, path, O_WRONLY, fd);
    });

    ptraceModule.onSyscall(SYS_link, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[1]);
        handle(state, path, O_WRONLY);
    });

    ptraceModule.onSyscall(SYS_linkat, [&](ProcessState& state) {
        int fd = state.syscall.args[2];
        fs::path path = state.readString((void*)state.syscall.args[3]);
        handleAt(state, path, O_WRONLY, fd);
    });

    ptraceModule.onSyscall(SYS_unlink, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        handle(state, path, O_WRONLY);
    });

    ptraceModule.onSyscall(SYS_unlinkat, [&](ProcessState& state) {
        int fd = state.syscall.args[0];
        fs::path path = state.readString((void*)state.syscall.args[1]);
        handleAt(state, path, O_WRONLY, fd);
    });

    ptraceModule.onSyscall(SYS_symlink, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[1]);
        handle(state, path, O_WRONLY);
    });

    ptraceModule.onSyscall(SYS_symlinkat, [&](ProcessState& state) {
        int fd = state.syscall.args[1];
        fs::path path = state.readString((void*)state.syscall.args[2]);
        handleAt(state, path, O_WRONLY, fd);
    });

    ptraceModule.onSyscall(SYS_chmod, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        handle(state, path, O_WRONLY);
    });

    ptraceModule.onSyscall(SYS_chown, [&](ProcessState& state) {
        fs::path path = state.readString((void*)state.syscall.args[0]);
        handle(state, path, O_WRONLY);
    });
}

FilesystemAction FilesystemModule::getAction(const fs::path& path, pid_t pid) const {
    fs::path x = path;

    if (path.is_relative()) {
        std::stringstream ss;
        ss << "/proc/" << pid << "/cwd";
        auto cwd = readLink(ss.str());
        x = (cwd / path).lexically_normal();
    }

//    std::cerr << "getAction: " << x << std::endl;

    for (const auto& rule : rules) {
        auto p = x;
        while (p != "" && p != "/") {
            if (p == rule.path) {
                return rule.action;
            }
            p = p.parent_path();
        }
    }
    
    return FilesystemAction::Allow;
}

void FilesystemModule::handle(ProcessState& state, const fs::path& path, unsigned flags) const {
//    std::cerr << "handle: " << path << std::endl;
    auto act = getAction(path, state.pid);
    if (act == FilesystemAction::Deny) {
        state.syscall.result = -EACCES;
    }
    if (flags & (O_CREAT | O_RDWR | O_WRONLY)) { 
        if (act != FilesystemAction::Allow) {
            state.syscall.result = -EACCES;
        }
    }
}

void FilesystemModule::handleAt(ProcessState& state, const fs::path& path, unsigned flags, int fd) const {
//    std::cerr << "handleAt: " << path << std::endl;
    fs::path base;
    if (fd != AT_FDCWD) {
        std::stringstream ss;
        ss << "/proc/" << state.pid << "/fd/" << fd;
        base = readLink(ss.str());
    }
    handle(state, base / path, flags);
}

