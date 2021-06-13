#include <sys/resource.h>

int main(int argc, char *argv[]) {
    rlimit limit{5, 5};
    prlimit(RLIMIT_CPU, RLIMIT_CPU, &limit, nullptr);
}
