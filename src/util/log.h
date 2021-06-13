#pragma once

#include <pthread.h>
#include <string>

class MultiprocessLog {
private:
    static std::string filename;
    static pthread_mutex_t* mutex;

public:
    static void init(const std::string& file);
    static void close();
    static void log(const std::string& message);
};
