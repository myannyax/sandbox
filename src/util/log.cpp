#include "log.h"

#include <iostream>
#include <sys/mman.h>
#include <pthread.h>
#include <fstream>

std::string MultiprocessLog::filename;
pthread_mutex_t* MultiprocessLog::mutex = nullptr;

void MultiprocessLog::init(const std::string& file = "") {
    mutex = reinterpret_cast<pthread_mutex_t*>(mmap(
        nullptr, sizeof(pthread_mutex_t),
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
        -1, 0
    ));
    if (mutex == MAP_FAILED) {
        throw std::runtime_error("Failed to allocate shared memory for mutex");
    }

    pthread_mutexattr_t attrmutex;
    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attrmutex);

    filename = file;
    if (!filename.empty()) {
        std::ofstream os;
        os.open(filename, std::ofstream::out | std::ofstream::trunc);
        os.close();
    }
}

void MultiprocessLog::close() {
    if (mutex != nullptr) {
        munmap(mutex, sizeof(pthread_mutex_t));
        mutex = nullptr;
    }
}

void MultiprocessLog::log(const std::string &message) {
    pthread_mutex_lock(mutex);
    if (!filename.empty()) {
        std::ofstream os;
        os.open(filename, std::ios_base::app);
        os << message << "\n";
        os.close();
    } else {
        std::cout << message << "\n";
    }
    pthread_mutex_unlock(mutex);
}
