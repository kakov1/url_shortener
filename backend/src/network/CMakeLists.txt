add_library(network
    server/server.cpp
    session/http_session.cpp
)

target_include_directories(network
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(network
    PUBLIC
        core
        logic
        concurrency
        Boost::system
        nlohmann_json::nlohmann_json
)