add_library(concurrency queue/queue.cpp threadpool/threadpool.cpp)

target_include_directories(concurrency
    PUBLIC 
    queue
    ../network/session
    ../core
    ../logic
)
