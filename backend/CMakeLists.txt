cmake_minimum_required(VERSION 3.16)
project(url_shortener)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(server
    src/main.cpp
    src/network/server.cpp
    src/shortener/shortener.cpp
    src/core/safequeue.cpp
    src/core/threadpool.cpp
)

target_include_directories(server PRIVATE src)

target_link_libraries(server Boost::system)