add_executable(unit_tests
    unit/unit_tests.cpp
    unit/test_url_service.cpp
    unit/test_queue.cpp
    unit/test_config.cpp
)

target_include_directories(unit_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(unit_tests
    PRIVATE
        logic
        config
        concurrency
        in_memory_repositories
        Boost::program_options
        Threads::Threads
        gtest
)

include(GoogleTest)
gtest_discover_tests(unit_tests)