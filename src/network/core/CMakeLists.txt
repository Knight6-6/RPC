cmake_minimum_required(VERSION 3.10)
project(core)

set(CMAKE_CXX_STANDARD 17)

add_library(core_lib 
    server.cpp 
    event_loop.cpp
    net_session.cpp
)

target_link_libraries(core_lib PUBLIC utils_lib)



