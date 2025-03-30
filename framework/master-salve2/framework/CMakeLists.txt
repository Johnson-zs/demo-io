cmake_minimum_required(VERSION 3.14)

set(FRAMEWORK_SOURCES
    src/plugin_loader.cpp
    src/message.cpp
    src/common.cpp
    src/worker_base.cpp
)

set(FRAMEWORK_HEADERS
    include/framework/plugin.h
    include/framework/message.h
    include/framework/plugin_loader.h
    include/framework/common.h
    include/framework/worker_base.h
    include/framework/worker_factory.h
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

# 创建框架库
add_library(framework SHARED ${FRAMEWORK_SOURCES} ${FRAMEWORK_HEADERS})

target_include_directories(framework 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(framework
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
)

# 创建Worker进程可执行文件
add_executable(worker_process
    src/worker_process.cpp
)

target_link_libraries(worker_process
    PRIVATE
        framework
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
)

# 设置输出目录
set_target_properties(worker_process PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 安装规则
install(TARGETS framework
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(TARGETS worker_process
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include) 
