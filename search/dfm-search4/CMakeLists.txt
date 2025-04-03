cmake_minimum_required(VERSION 3.14)

project(dfm6-search VERSION 0.1 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core)

# 构建库
add_subdirectory(src)

# 构建示例
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(example)
    add_subdirectory(examples/desktop_search_demo)
endif()

# 安装头文件
install(DIRECTORY include/dfm6-search DESTINATION include) 
