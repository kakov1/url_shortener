add_library(network server/server.cpp session/session.cpp)

target_include_directories(network
    PUBLIC 
    ../concurrency/threadpool
    ../concurrency/queue
    ../core
    ../logic
)
