cmake_minimum_required(VERSION 3.10)

project(naming)

set(CMAKE_CXX_STANDARD 17)

add_library (naming_lib
    naming_server.cpp
    naming_table.cpp
)

target_link_libraries(naming_lib PUBLIC utils_lib ${Protobuf_LIBRARIES})