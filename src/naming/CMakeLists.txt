cmake_minimum_required(VERSION 3.10)

project(naming)

set(CMAKE_CXX_STANDARD 17)

add_library (etcd_client
etcd_client.cpp
)

target_link_libraries(etcd_client PUBLIC utils ${Protobuf_LIBRARIES})