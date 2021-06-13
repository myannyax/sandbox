#include "MountNamespace.h"
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/syscall.h>

namespace fs = std::filesystem;

static const std::string rootfsDataPath = "/";
static const std::string rootfsMountPath = ".rootfs";
static std::string upper = ".upper";
static std::string work = ".work";
static std::string put_old = ".put_old";

void MountNamespace::apply(Runner &runner) {
    upper = (fs::current_path() / upper);
    work = (fs::current_path() / work);
    auto mount_root = config.config["mount_root"].as<bool>();
    int flags = CLONE_NEWNS;
    auto pid_ns = config.config["pid_namespace"].as<bool>();
    if (pid_ns) {
        flags |= CLONE_NEWPID;
    }
    runner.setUnshareFlags(flags);

    runner.addHook(Hook::BeforeExec, [&](pid_t) {
        auto curDir = std::filesystem::current_path();
        if (mount_root) {
            if (mkdir(upper.data(), 0777) && errno != EEXIST) {
                throw std::runtime_error{"Failed to mkdir upper"};
            }

            if (mkdir(work.data(), 0777) && errno != EEXIST) {
                throw std::runtime_error{"Failed to mkdir work"};
            }

            if (mkdir(rootfsMountPath.data(), 0777) && errno != EEXIST) {
                throw std::runtime_error{"Failed to mkdir rootfsMountPath"};
            }

            std::stringstream ss;
            ss << "lowerdir=" << rootfsDataPath << ",upperdir=" << upper << ",workdir=" << work;
            if (mount("overlay", rootfsMountPath.data(), "overlay", 0, ss.str().data())) {
                throw std::runtime_error{"Failed to mount overlayfs"};
            }

            if (chdir(rootfsMountPath.data())) {
                throw std::runtime_error{"Failed to chdir to rootfsMountPath"};
            }

            if (mkdir(put_old.data(), 0777) && errno != EEXIST) {
                throw std::runtime_error{"Failed to mkdir put_old"};
            }

            if (syscall(SYS_pivot_root, ".", put_old.data())) {
                throw std::runtime_error{"Failed to pivot_root"};
            }
        }

        if (pid_ns) {

            if (mkdir("/proc", 0555) && errno != EEXIST) {
                throw std::runtime_error{"Failed to mkdir /proc"};
            }

            if (mount("proc", "/proc", "proc", 0, "")) {
                throw std::runtime_error{"Failed to mount /proc"};
            }
        }

        if (mount_root) {
            if (umount2(put_old.data(), MNT_DETACH)) {
                throw std::runtime_error{"Failed to umount put_old"};
            }
        }

        if (setgid(0) == -1) {
            throw std::runtime_error{"Failed to setgid"};
        }

        if (setuid(0) == -1) {
            throw std::runtime_error{"Failed to setuid"};
        }

        if (mount_root) {
            if (chdir(curDir.c_str())) {
                throw std::runtime_error{"Failed to chdir to old cwd"};
            }
        }
    });

    runner.addHook(Hook::Cleanup, [&](pid_t) {
        std::filesystem::remove_all(upper);
        std::filesystem::remove_all(work);
        std::filesystem::remove_all(rootfsMountPath);
    });
}
