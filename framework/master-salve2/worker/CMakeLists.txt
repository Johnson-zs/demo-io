cmake_minimum_required(VERSION 3.16)

set(WORKER_SOURCES
    src/main.cpp
    src/worker.cpp
    src/plugin_manager.cpp
)

set(WORKER_HEADERS
    include/worker/worker.h
    include/worker/plugin_manager.h
)

add_executable(worker ${WORKER_SOURCES} ${WORKER_HEADERS})

target_include_directories(worker 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(worker
    PRIVATE
        framework
        Qt6::Core
        Qt6::Network
)

# 设置插件目录相对路径
target_compile_definitions(worker PRIVATE
    PLUGINS_DIR="${CMAKE_INSTALL_PREFIX}/lib/plugins"
)

# 安装规则
install(TARGETS worker
    RUNTIME DESTINATION bin
) 