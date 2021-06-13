#include <signal.h>

int main(int argc, char *argv[]) {
    kill(1, SIGKILL);
}
