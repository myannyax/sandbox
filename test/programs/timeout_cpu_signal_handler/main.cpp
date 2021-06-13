#include <iostream>
#include <unistd.h>
#include <chrono>
#include <signal.h>

int main(int argc, char *argv[]) {
    signal(SIGXCPU, [](int){});

    int size = std::stoi(argv[1]);

    auto start_time = std::chrono::high_resolution_clock::now();;
    while (true) {
        auto cur_time = std::chrono::high_resolution_clock::now();;
        auto time_passed = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - start_time);
        if (time_passed.count() > size * 1000) {
            break;
        }
    }

    std::cout << "Hey" << std::endl;
}
