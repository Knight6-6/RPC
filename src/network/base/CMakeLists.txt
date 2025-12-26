cmake_minimum_required(VERSION 3.10)

project(base)

set(CMAKE_CXX_STANDARD 17)

add_library(base_lib client.cpp)

target_include_directories(base_lib PUBLIC ../protocols)
