cmake_minimum_required(VERSION 3.20)

project(RPC)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src/network)
add_subdirectory(src/rpc)
