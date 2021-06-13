#include "UserNamespace.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <unistd.h>

static void writeToFile(const std::string &path, const std::string &line) {
    std::ofstream file;
    file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    file.open(path);
    file << line;
    file.close();
}

void UserNamespace::apply(Runner &runner) {
    runner.setUnshareFlags(CLONE_NEWUSER);

    runner.addHook(Hook::ParentBeforeExec, [&](pid_t pid) {
        char path[100];
        char line[100];

        auto uid = getuid();

        sprintf(path, "/proc/%d/uid_map", pid);
        sprintf(line, "0 %d 1\n", uid);
        writeToFile(path, line);

        sprintf(path, "/proc/%d/setgroups", pid);
        sprintf(line, "deny");
        writeToFile(path, line);

        sprintf(path, "/proc/%d/gid_map", pid);
        sprintf(line, "0 %d 1\n", uid);
        writeToFile(path, line);
    });
}
