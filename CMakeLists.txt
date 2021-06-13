cmake_minimum_required(VERSION 3.12)
project("sandbox" LANGUAGES C CXX VERSION "0.1")
include(cmake/CPM.cmake)

add_executable(my_sandbox
    src/main.cpp
    src/runner.cpp
    src/util/config.cpp
    src/util/units.cpp
    src/util/log.cpp
    src/modules/exec.cpp
    src/modules/ptrace.cpp
    src/modules/MountNamespace.cpp
    src/modules/UserNamespace.cpp
    src/modules/limit.cpp
    src/modules/priority.cpp
)

CPMAddPackage("gh:jarro2783/cxxopts#dd45a0801c99d62109aaa29f8c410ba8def2fbf2")
CPMAddPackage("gh:jbeder/yaml-cpp#yaml-cpp-0.6.3@0.6.3")

find_package(Threads REQUIRED)

target_compile_options(my_sandbox PRIVATE -Wall -Wextra -pedantic)
target_compile_features(my_sandbox PRIVATE cxx_std_20)
target_link_libraries(my_sandbox PRIVATE
    Threads::Threads
    cxxopts::cxxopts
    yaml-cpp
)
target_include_directories(my_sandbox PRIVATE src)
