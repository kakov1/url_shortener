add_library(user_repository
    in_memory_user_repository.cpp
)

target_include_directories(user_repository
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(user_repository
    PUBLIC
        entities
)