cmake_minimum_required(VERSION 3.16)
project(PPTThumbnailViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)

# 设置Qt6的自动化工具
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/thumbnailgenerator.cpp
    src/thumbnailwidget.cpp
)

set(HEADERS
    src/mainwindow.h
    src/thumbnailgenerator.h
    src/thumbnailwidget.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接Qt6库
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Concurrent)

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE src)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif() 