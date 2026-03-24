cmake_minimum_required(VERSION 3.16)
project(url_shortener)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" OFF)

find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)
# find_package(PQXX REQUIRED)

add_subdirectory(src/entities)
add_subdirectory(src/logic)
add_subdirectory(src/concurrency)
add_subdirectory(src/network)
add_subdirectory(src/repositories)
add_subdirectory(src/app/config)

add_executable(shortener
    src/app/main.cpp
)

target_include_directories(shortener
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(shortener
    PRIVATE
        logic
        config
        concurrency
        network
        postgres_repositories
        Boost::system
        Boost::program_options
        Threads::Threads
)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
