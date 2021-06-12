//
// Created by Maria.Filipanova on 6/9/21.
//

#include "MountNamespace.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

static const std::string rootfsDataPath = "/";
static const std::string rootfsMountPath = "rootfs";
static const std::string upper = (fs::current_path() / ".upper");
static const std::string work = (fs::current_path() / ".work");

// TODO: change?
static void die(const char *fmt, ...) {
    va_list params;
    va_start(params, fmt);
    vfprintf(stderr, fmt, params);
    va_end(params);
    exit(1);
}

void MountNamespace::apply(Runner &runner) {
    runner.addHook(Hook::BeforeExec, [&](pid_t) {
        std::cout << getuid();

        if (mkdir(upper.data(), 0777) && errno != EEXIST)
            die("Failed to mkdir put_old %s: %m\n", upper.data());

        if (mkdir(work.data(), 0777) && errno != EEXIST)
            die("Failed to mkdir put_old %s: %m\n", work.data());

        std::stringstream ss;
        ss << "lowerdir=" << rootfsDataPath << ",upperdir=" << upper << ",workdir=" << work;
        if (mount("overlay", rootfsMountPath.data(), "overlay", 0, ss.str().data()))
            die("Failed to mount %s at %s: %m\n", rootfsDataPath.data(), rootfsMountPath.data());

        if (chdir(rootfsMountPath.data()))
            die("Failed to chdir to rootfs mounted at %s: %m\n", rootfsMountPath.data());

        const char *put_old = ".put_old";
        if (mkdir(put_old, 0777) && errno != EEXIST)
            die("Failed to mkdir put_old %s: %m\n", put_old);

        if (syscall(SYS_pivot_root, ".", put_old))
            die("Failed to pivot_root from %s to %s: %m\n", rootfsMountPath.data(), put_old);

        if (chdir("/"))
            die("Failed to chdir to new root: %m\n");

        if (mkdir("/proc", 0555) && errno != EEXIST)
            die("Failed to mkdir /proc: %m\n");

        if (mount("proc", "/proc", "proc", 0, ""))
            die("Failed to mount proc: %m\n");

        if (umount2(put_old, MNT_DETACH))
            die("Failed to umount put_old %s: %m\n", put_old);

        if (setgid(0) == -1)
            die("Failed to setgid: %m\n");
        if (setuid(0) == -1)
            die("Failed to setuid: %m\n");
    });

    runner.addHook(Hook::Cleanup, [&](pid_t) {
        std::filesystem::remove_all(upper);
        std::filesystem::remove_all(work);
    });
}
