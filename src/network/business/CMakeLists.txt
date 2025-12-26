cmake_minimum_required(VERSION 3.10)

project(business)

set(CMAKE_CXX_STANDARD 17)

add_library(business_lib 
    user_manager.cpp 
    busi_session.cpp
)

target_include_directories(business_lib PUBLIC ../protocols ../utils)
target_link_libraries(business_lib PUBLIC utils_lib)
