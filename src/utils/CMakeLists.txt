cmake_minimum_required(VERSION 3.10)
project(utils)

set(CMAKE_CXX_STANDARD 17)

add_library(utils_lib 
    logger.cpp 
    thread_pool.cpp 
    ring_buf.cpp 
    timer.cpp 
    connect_pool.cpp
)
