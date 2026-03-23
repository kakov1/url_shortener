add_library(logic
    url_service.cpp
    in_memory_url_repository.cpp
)

target_include_directories(logic
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(logic
    PUBLIC
        core
)