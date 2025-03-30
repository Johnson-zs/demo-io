cmake_minimum_required(VERSION 3.16)

set(EXAMPLE_SOURCES
    src/main.cpp
)

add_executable(example ${EXAMPLE_SOURCES})

target_link_libraries(example
    PRIVATE
        framework
        Qt6::Core
        Qt6::Network
)

# 安装规则
install(TARGETS example
    RUNTIME DESTINATION bin
) 