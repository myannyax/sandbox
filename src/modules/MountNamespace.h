//
// Created by Maria.Filipanova on 6/9/21.
//

#ifndef SANDBOX_MOUNTNAMESPACE_H
#define SANDBOX_MOUNTNAMESPACE_H

#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <wait.h>
#include <memory.h>
#include <syscall.h>
#include <errno.h>
#include "../runner.h"

struct MountNamespace {
    char* rootfs = "rootfs";

    void apply(Runner& runner);
};


#endif //SANDBOX_MOUNTNAMESPACE_H
