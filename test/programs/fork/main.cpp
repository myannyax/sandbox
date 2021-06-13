#include <unistd.h>
#include <sys/syscall.h>

int main(int argc, char *argv[]) {
    for (int i = 0; i < 11; i++) {
        int pid = syscall(SYS_fork);
        if (pid == 0) {
            return 0;
        }
    }
    return 0;
}
