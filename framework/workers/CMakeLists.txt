cmake_minimum_required(VERSION 3.10)
project(mini_nginx CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 添加编译选项
add_compile_options(-Wall -Wextra)

# 添加子目录
add_subdirectory(src) 