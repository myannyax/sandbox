cmake_minimum_required(VERSION 3.12)
project("test" LANGUAGES C CXX VERSION "0.1")

add_executable(main main.cpp)

target_compile_features(main PRIVATE cxx_std_20)
