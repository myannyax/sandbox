//
// Created by Maria.Filipanova on 6/9/21.
//

#ifndef SANDBOX_USERNAMESPACE_H
#define SANDBOX_USERNAMESPACE_H

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


struct UserNamespace {
    int uidToCopy;
    void apply(Runner& runner);
};


#endif //SANDBOX_USERNAMESPACE_H
