#include "UserNamespace.h"
#include <stdexcept>

#include <cstring>
#include <unistd.h>

static void writeToFile(const char *path, const char *line) {
    FILE *f = fopen(path, "w");

    if (f == NULL) {
        throw std::runtime_error{"Failed to open file " + std::string(path)};
    }

    if (fwrite(line, 1, strlen(line), f) < 0) {
        throw std::runtime_error{"Failed to write to file " + std::string(path)};
    }

    fclose(f);
}

void UserNamespace::apply(Runner &runner) {
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
