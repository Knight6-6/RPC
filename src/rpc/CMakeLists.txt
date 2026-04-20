cmake_minimum_required(VERSION 3.20)

project (rpc)

set(CMAKE_STANDARD 17)

add_library(rpc
    rpc_client.cpp
    rpc_server.cpp
    rpc_dispatcher.cpp
)

target_link_libraries(rpc PUBLIC 
    core 
    utils 
    ${Protobuf_LIBRARIES}
)