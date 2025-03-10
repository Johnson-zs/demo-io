cmake_minimum_required(VERSION 3.16)
project(file_search_demo VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Dtk6 REQUIRED COMPONENTS Widget)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)

add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    mainwindow.h
    searchengine.cpp
    searchengine.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Dtk6::Widget
    PkgConfig::Lucene
) 
