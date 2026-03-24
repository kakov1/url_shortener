add_library(postgres_repositories
    users/postgres_user_repository.cpp
    urls/postgres_url_repository.cpp
    postgres_connection_pool.cpp
)

target_include_directories(postgres_repositories
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(postgres_repositories
    PUBLIC
        entities
        pqxx
)