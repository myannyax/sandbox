//
// Created by Maria.Filipanova on 6/9/21.
//

#include "UserNamespace.h"
#include <stdexcept>

// TODO: change?
static void die(const char *fmt, ...) {
    va_list params;

    va_start(params, fmt);
    vfprintf(stderr, fmt, params);
    va_end(params);
    throw std::runtime_error{"oh god oh fuck"};
}

// TODO: same as above
static void write_file(char path[100], char line[100]) {
    FILE *f = fopen(path, "w");

    if (f == NULL) {
        die("Failed to open file %s: %m\n", path);
    }

    if (fwrite(line, 1, strlen(line), f) < 0) {
        die("Failed to write to file %s:\n", path);
    }

    if (fclose(f) != 0) {
        //die("Failed to close file %s: %m\n", path);
    }
}

void UserNamespace::apply(Runner &runner) {
    runner.addHook(Hook::ParentBeforeExec, [&](pid_t pid) {
        char path[100];
        char line[100];

        int uid = 1000;

        sprintf(path, "/proc/%d/uid_map", pid);
        sprintf(line, "0 %d 1\n", uid);
        write_file(path, line);

        sprintf(path, "/proc/%d/setgroups", pid);
        sprintf(line, "deny");
        write_file(path, line);

        sprintf(path, "/proc/%d/gid_map", pid);
        sprintf(line, "0 %d 1\n", uid);
        write_file(path, line);
    });
}
