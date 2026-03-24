add_library(config
    config.cpp
)

target_include_directories(config
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)
