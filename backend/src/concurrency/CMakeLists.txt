add_library(concurrency
    queue/queue.cpp
    threadpool/threadpool.cpp
)

target_include_directories(concurrency
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(concurrency
    PUBLIC
        core
        logic
        Threads::Threads
)
