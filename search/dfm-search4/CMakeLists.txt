cmake_minimum_required(VERSION 3.16)
project(dfm6-search VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# 查找Qt6包
find_package(Qt6 REQUIRED COMPONENTS Core)

# 添加库子目录
add_subdirectory(src)

# 添加示例子目录
add_subdirectory(example)

# 安装头文件
install(DIRECTORY include/dfm6-search DESTINATION include) 