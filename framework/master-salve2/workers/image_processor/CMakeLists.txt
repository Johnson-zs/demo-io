cmake_minimum_required(VERSION 3.14)

project(image_processor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

set(IMAGE_PROCESSOR_SOURCES
    src/image_processor.cpp
    src/image_processor_factory.cpp
)

set(IMAGE_PROCESSOR_HEADERS
    include/image_processor.h
    include/image_processor_factory.h
)

# 添加JSON元数据文件
set(JSON_FILES
    include/image_processor.json
)

# 创建Worker插件
add_library(image_processor MODULE
    ${IMAGE_PROCESSOR_SOURCES}
    ${IMAGE_PROCESSOR_HEADERS}
    ${JSON_FILES}
)

# 设置插件输出目录
set_target_properties(image_processor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${WORKER_PLUGIN_OUTPUT_DIRECTORY}"
)

# 包含目录
target_include_directories(image_processor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/framework/include
)

# 链接库
target_link_libraries(image_processor
    PRIVATE
    framework
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
)

# 安装规则
install(TARGETS image_processor
    LIBRARY DESTINATION workers
)

install(FILES ${JSON_FILES}
    DESTINATION workers
)

# 拷贝文件到构建目录
add_custom_command(TARGET image_processor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${WORKER_PLUGIN_OUTPUT_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/include/image_processor.json"
            "${WORKER_PLUGIN_OUTPUT_DIRECTORY}"
) 