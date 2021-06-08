cmake_minimum_required(VERSION 3.12)
project("sandbox" LANGUAGES C CXX VERSION "0.1")
include(cmake/CPM.cmake)

add_executable(sandbox
    src/main.cpp
    src/runner.cpp
)

find_package(Threads REQUIRED)

target_compile_options(sandbox PRIVATE -Wall -Wextra -pedantic)
target_compile_features(sandbox PRIVATE cxx_std_20)
target_link_libraries(sandbox PRIVATE Threads::Threads)
