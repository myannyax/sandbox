#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int size = std::stoi(argv[1]);
    sleep(size);

    std::cout << "Hey" << std::endl;
}
