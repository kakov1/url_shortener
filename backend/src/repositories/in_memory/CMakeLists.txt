add_library(in_memory_repositories
    users/in_memory_user_repository.cpp
    urls/in_memory_url_repository.cpp
)

target_include_directories(in_memory_repositories
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(in_memory_repositories
    PUBLIC
        entities
)
