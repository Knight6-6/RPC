cmake_minimum_required(VERSION 3.20)

project (rpc)

set(CMAKE_STANDARD 17)

add_library(rpc_lib
    rpc_client.cpp
    rpc_server.cpp
    rpc_dispatcher.cpp
)

target_link_libraries(rpc_lib PUBLIC 
    core_lib 
    utils_lib 
    ${Protobuf_LIBRARIES}
)