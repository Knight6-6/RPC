cmake_minimum_required(VERSION 3.20)

project(protocols)

set(CMAKE_CXX_STANDARD 17)

add_library(protocols_lib codec.cpp)

target_link_libraries(protocols_lib PUBLIC )