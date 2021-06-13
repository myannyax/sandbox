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
    static void log_info(const std::string& message);
    static void log_error(const std::string& message);
    static void log_fatal(const std::string& message);

private:
    static void log_impl(const std::string& message);
};
