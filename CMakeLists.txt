cmake_minimum_required(VERSION 3.20)

project(RPC)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include)


find_package(Protobuf REQUIRED)
find_package(etcd-cpp-api REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})

add_subdirectory(src/utils)
add_subdirectory(src/naming)
add_subdirectory(src/network)
add_subdirectory(src/rpc)


