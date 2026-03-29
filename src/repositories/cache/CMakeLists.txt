add_library(cache
    redis/redis_cache.cpp
)

target_include_directories(cache
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(cache
    PUBLIC
        redis++
        hiredis
)