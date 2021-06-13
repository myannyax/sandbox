#include "units.h"

#include <sstream>

namespace {
    void to_lower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), [](int c) {
            if (c >= 'A' && c <= 'Z') { // kek
                return 'a' + (c - 'A');
            }
            return c;
        });
    }
}

int parse_time(const std::string &str) {
    std::istringstream is{str};
    int result = 0;

    int t;
    while (is >> t) {
        std::string unit;
        is >> unit;
        to_lower(unit);
        int scale = 1;
        if (unit == "m") {
            scale = 60;
        } else if (unit == "h") {
            scale = 60 * 60;
        } else if (unit == "d") {
            scale = 24 * 60 * 60;
        }
        result += t * scale;
    }
    return result;
}

int parse_memory_size(const std::string &str) {
    std::istringstream is{str};
    int result = 0;

    int s;
    while (is >> s) {
        std::string unit;
        is >> unit;
        to_lower(unit);
        int scale = 1;
        if (unit == "kb") {
            scale = 1024;
        } else if (unit == "mb") {
            scale = 1024 * 1024;
        } else if (unit == "gb") {
            scale = 1024 * 1024 * 1024;
        }
        result += s * scale;
    }
    return result;
}
