add_library(logic
    url_service.cpp
)

target_include_directories(logic
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(logic
    PUBLIC
        entities
)