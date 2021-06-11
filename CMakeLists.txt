cmake_minimum_required(VERSION 3.12)
project("sandbox" LANGUAGES C CXX VERSION "0.1")
include(cmake/CPM.cmake)

add_executable(my_sandbox
    src/main.cpp
    src/runner.cpp
    src/modules/exec.cpp
    src/modules/wait.cpp
    src/modules/MountNamespace.cpp
    src/modules/UserNamespace.cpp
    src/util/config.cpp
)

CPMAddPackage("gh:jbeder/yaml-cpp#yaml-cpp-0.6.3@0.6.3")

find_package(Threads REQUIRED)

target_compile_options(my_sandbox PRIVATE -Wall -Wextra -pedantic)
target_compile_features(my_sandbox PRIVATE cxx_std_20)
target_link_libraries(my_sandbox PRIVATE Threads::Threads yaml-cpp)
