cmake_minimum_required(VERSION 3.14)

project(worker_process LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(PROJECT_SOURCES
    src/main.cpp
)

# 创建可执行文件
add_executable(worker_process
    ${PROJECT_SOURCES}
)

# 包含目录
target_include_directories(worker_process
    PRIVATE
    ${CMAKE_SOURCE_DIR}/framework/include
)

# 链接Qt库和框架库
target_link_libraries(worker_process
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    framework
)

# 安装
install(TARGETS worker_process
    RUNTIME DESTINATION bin
) 