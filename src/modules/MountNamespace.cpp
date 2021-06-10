//
// Created by Maria.Filipanova on 6/9/21.
//

#include "MountNamespace.h"

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
        const char *mnt = rootfs;

        if (mount(rootfs, mnt, "ext4", MS_BIND, ""))
            die("Failed to mount %s at %s: %m\n", rootfs, mnt);

        if (chdir(mnt))
            die("Failed to chdir to rootfs mounted at %s: %m\n", mnt);

        const char *put_old = ".put_old";
        if (mkdir(put_old, 0777) && errno != EEXIST)
            die("Failed to mkdir put_old %s: %m\n", put_old);

        if (syscall(SYS_pivot_root, ".", put_old))
            die("Failed to pivot_root from %s to %s: %m\n", rootfs, put_old);

        if (chdir("/"))
            die("Failed to chdir to new root: %m\n");

        if (mkdir("/proc", 0555) && errno != EEXIST)
            die("Failed to mkdir /proc: %m\n");

        if (mount("proc", "/proc", "proc", 0, ""))
            die("Failed to mount proc: %m\n");

        if (umount2(put_old, MNT_DETACH))
            die("Failed to umount put_old %s: %m\n", put_old);

        // TODO: understand that this is & change probably but I haven't watch lecture about this yet
        // Assuming, 0 in the current namespace maps to
        // a non-privileged UID in the parent namespace,
        // drop superuser privileges if any by enforcing
        // the exec'ed process runs with UID 0.
        if (setgid(0) == -1)
            die("Failed to setgid: %m\n");
        if (setuid(0) == -1)
            die("Failed to setuid: %m\n");
    });
}