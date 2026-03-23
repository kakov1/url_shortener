cmake_minimum_required(VERSION 3.16)
project(url_shortener)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" OFF)

find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src/core)
add_subdirectory(src/logic)
add_subdirectory(src/concurrency)
add_subdirectory(src/network)

add_executable(server
    src/main.cpp
)

target_link_libraries(server
    PRIVATE
        core
        logic
        concurrency
        network
        Boost::system
        Boost::program_options
        Threads::Threads
)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
