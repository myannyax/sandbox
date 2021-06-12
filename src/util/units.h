#pragma once

#include <boost/algorithm/string/case_conv.hpp>
#include <string>
#include <sstream>

int parse_time(const std::string &str) {
    std::istringstream is{str};
    int result = 0;

    int t;
    while (is >> t) {
        std::string unit;
        is >> unit;
        boost::to_lower(unit);
        int scale = 1;
        if (unit == "s") {
            scale = 1000;
        } else if (unit == "m") {
            scale = 60 * 1000;
        } else if (unit == "h") {
            scale = 60 * 60 * 1000;
        } else if (unit == "d") {
            scale = 24 * 60 * 60 * 1000;
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
        boost::to_lower(unit);
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
