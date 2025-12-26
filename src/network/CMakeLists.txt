cmake_minimum_required(VERSION 3.10)

project(server)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
link_directories(${PQXX_LIBRARY_DIRS})

add_subdirectory(base)
add_subdirectory(utils)
add_subdirectory(business)                              
add_subdirectory(core)

add_executable(server main.cpp)

target_include_directories(server
    PRIVATE
        core
        ${PQXX_INCLUDE_DIRS}
)

target_link_libraries(server 
    core_lib 
    business_lib 
    utils_lib
    ${PQXX_LIBRARIES} 
)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build: enabling sanitizers")
    add_compile_options(
        -O1
        -g
        -Wall
        -Wextra
        -Wpedantic
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    add_link_options(-fsanitize=address,undefined)
endif()