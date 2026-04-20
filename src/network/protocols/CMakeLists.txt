cmake_minimum_required(VERSION 3.10)

project(protocols)

set(CMAKE_CXX_STANDARD 17)

add_library(network_protocols 
    codec.cpp
)

target_link_libraries(network_protocols PUBLIC 
    utils 
)



