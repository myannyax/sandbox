#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int pid = fork();
    if (pid == 0) {
        kill(1, SIGKILL);
    }
}
