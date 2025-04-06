cmake_minimum_required(VERSION 3.20)
project(error_handler VERSION 1.0)

# 设置C++标准为C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 添加包含目录
include_directories(include)

# 添加源文件
file(GLOB SOURCES "src/*.cpp")

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 检查编译器支持
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        message(FATAL_ERROR "GCC version must be at least 12.0 for C++23 support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
        message(FATAL_ERROR "Clang version must be at least 15.0 for C++23 support")
    endif()
endif() 