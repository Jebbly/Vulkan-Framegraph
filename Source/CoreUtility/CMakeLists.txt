set(CORE_UTILITY_SRC
    Logging.cpp
)

add_library(CoreUtility STATIC ${CORE_UTILITY_SRC})

target_link_libraries(CoreUtility PUBLIC spdlog)
