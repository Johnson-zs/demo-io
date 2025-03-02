cmake_minimum_required(VERSION 3.16)
project(master-slave-demo VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Network
)

add_subdirectory(common)
add_subdirectory(src/master)
add_subdirectory(src/slave) 