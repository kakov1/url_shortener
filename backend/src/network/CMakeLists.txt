add_library(network server/server.cpp session/http_session.cpp)

target_include_directories(network
    PUBLIC 
    server
    session
    ../concurrency/threadpool
    ../concurrency/queue
    ../core
    ../logic
)

target_link_libraries(network
    PUBLIC
    concurrency
    logic
    Boost::system
)
