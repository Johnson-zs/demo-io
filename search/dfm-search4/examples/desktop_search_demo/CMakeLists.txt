cmake_minimum_required(VERSION 3.14)

project(desktop_search_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(desktop_search_demo
    main.cpp
)

target_link_libraries(desktop_search_demo
    PRIVATE
        Qt6::Core
        dfm6-search
) 
