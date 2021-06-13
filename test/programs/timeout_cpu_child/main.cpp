#include <iostream>
#include <unistd.h>
#include <chrono>

int main(int argc, char *argv[]) {
    int size = std::stoi(argv[1]);
    int pid = fork();
    if (pid == 0) {
        pid = fork();
        if (pid == 0) {
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
    }
}
