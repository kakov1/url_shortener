cmake_minimum_required(VERSION 3.16)
project(url_shortener)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

add_subdirectory(src/network)
add_subdirectory(src/concurrency)
add_subdirectory(src/logic)

add_executable(server
    src/main.cpp
)

target_include_directories(server PRIVATE src/network/server)

target_link_libraries(server PRIVATE network concurrency logic Boost::system)
