cmake_minimum_required(VERSION 3.16)

project(OfficePreview VERSION 1.0.0 LANGUAGES CXX)

# 设置C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用Qt的MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找Qt6包
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# 查找Poppler-Qt6
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_QT6 REQUIRED poppler-qt6)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${POPPLER_QT6_INCLUDE_DIRS})

# 源文件
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/OfficePreviewWidget.cpp
    src/OfficeConverter.cpp
)

# 头文件
set(HEADERS
    src/MainWindow.h
    src/OfficePreviewWidget.h
    src/OfficeConverter.h
)

# 创建可执行文件
add_executable(OfficePreview ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(OfficePreview 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Gui
    ${POPPLER_QT6_LIBRARIES}
)

# 编译选项
target_compile_options(OfficePreview PRIVATE ${POPPLER_QT6_CFLAGS_OTHER})

# 设置输出目录
set_target_properties(OfficePreview PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 复制Qt6库到输出目录（用于部署）
if(WIN32)
    add_custom_command(TARGET OfficePreview POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Core>
        $<TARGET_FILE:Qt6::Widgets>
        $<TARGET_FILE:Qt6::Gui>
        $<TARGET_FILE_DIR:OfficePreview>
    )
endif()

# 添加测试程序
add_executable(TestConverter test_converter.cpp src/OfficeConverter.cpp)
target_link_libraries(TestConverter
    Qt6::Core
    ${POPPLER_QT6_LIBRARIES}
)
target_compile_options(TestConverter PRIVATE ${POPPLER_QT6_CFLAGS_OTHER})
set_target_properties(TestConverter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
