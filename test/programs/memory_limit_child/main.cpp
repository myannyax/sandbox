#include <unistd.h>
#include <string>

int main(int, char *argv[]) {
    int size = std::stoi(argv[1]);
    int pid = fork();
    if (pid == 0) {
        pid = fork();
        if (pid == 0) {
            new int[size]();
        }
    }
}
