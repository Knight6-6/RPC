cmake_minimum_required(VERSION 3.10)
project(core)

set(CMAKE_CXX_STANDARD 17)

add_library(network_core 
    client.cpp
    event_loop_group.cpp
    event_loop.cpp
    net_session.cpp
    server.cpp 
)

target_link_libraries(network_core PUBLIC 
    utils 
    network_protocols
    ${Protobuf_LIBRARIES}
)



