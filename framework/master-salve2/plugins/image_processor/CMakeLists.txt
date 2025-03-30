cmake_minimum_required(VERSION 3.14)

project(image_processor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

set(PROJECT_SOURCES
    src/image_processor.cpp
    include/image_processor.h
    src/image_processor.json
)

# 添加插件库
add_library(image_processor SHARED
    ${PROJECT_SOURCES}
)

# 使插件和程序使用相同的输出目录
set_target_properties(image_processor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# 包含目录
target_include_directories(image_processor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/framework/include
)

# 链接Qt库
target_link_libraries(image_processor
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    framework
)

# 安装插件
install(TARGETS image_processor
    LIBRARY DESTINATION plugins
)

install(FILES src/image_processor.json
    DESTINATION plugins
)

# 拷贝文件到构建目录
add_custom_command(TARGET image_processor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins"
    COMMAND ${CMAKE_COMMAND} -E copy 
            "${CMAKE_CURRENT_SOURCE_DIR}/src/image_processor.json"
            "${CMAKE_BINARY_DIR}/plugins"
) 