cmake_minimum_required(VERSION 3.10)
project(core)

set(CMAKE_CXX_STANDARD 17)

add_library(core_lib 
    server.cpp 
    io_server.cpp
    net_session.cpp
)

target_include_directories(core_lib PUBLIC ../base ../utils ../protocols ../business)
target_link_libraries(core_lib PUBLIC base_lib utils_lib business_lib)



