cmake_minimum_required(VERSION 3.14)

project(test_client LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加必要的Qt组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

set(PROJECT_SOURCES
    src/main.cpp
)

# 创建可执行文件
add_executable(test_client
    ${PROJECT_SOURCES}
)

# 设置输出目录
set_target_properties(test_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 包含目录
target_include_directories(test_client
    PRIVATE
    ${CMAKE_SOURCE_DIR}/framework/include
)

# 链接库
target_link_libraries(test_client
    PRIVATE
    framework
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)

# 安装规则
install(TARGETS test_client
    RUNTIME DESTINATION bin
) 