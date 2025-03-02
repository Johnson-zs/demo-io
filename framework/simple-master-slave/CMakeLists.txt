cmake_minimum_required(VERSION 3.16)
project(simple-master-slave VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
)

# 添加服务器子项目
add_executable(server
    server/main.cpp
    server/server.h
    server/server.cpp
)

target_link_libraries(server PRIVATE
    Qt6::Core
    Qt6::Network
)

# 添加客户端子项目
add_executable(client
    client/main.cpp
    client/client.h
    client/client.cpp
)

target_link_libraries(client PRIVATE
    Qt6::Core
    Qt6::Network
)
