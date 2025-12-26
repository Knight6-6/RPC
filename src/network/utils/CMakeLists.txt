cmake_minimum_required(VERSION 3.10)
project(utils)

set(CMAKE_CXX_STANDARD 17)

add_library(utils_lib 
    logger.cpp 
    object_pool.hpp 
    thread_pool.cpp 
    ring_buf.cpp 
    timer.cpp 
    connect_pool.cpp
)

target_include_directories(utils_lib PUBLIC ../base ../protocols)
target_link_libraries(utils_lib PUBLIC base_lib )
