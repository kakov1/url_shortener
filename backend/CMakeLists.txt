cmake_minimum_required(VERSION 3.16)
project(url_shortener)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(nlohmann_json REQUIRED)

add_executable(server
    src/main.cpp

    src/network/server/server.cpp
    src/network/session/http_session.cpp

    src/concurrency/threadpool/threadpool.cpp

    src/logic/in_memory_url_repository.cpp
    src/logic/url_service.cpp
)

target_include_directories(server PRIVATE
    src/core
    src/logic
    src/network/server
    src/network/session
    src/concurrency/queue
    src/concurrency/threadpool
)

target_link_libraries(server PRIVATE
    Boost::system
    nlohmann_json::nlohmann_json
    Boost::program_options
    pthread
)