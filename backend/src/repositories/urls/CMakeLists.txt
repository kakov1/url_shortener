add_library(url_repository
    in_memory_url_repository.cpp
)

target_include_directories(url_repository
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(url_repository
    PUBLIC
        entities
)