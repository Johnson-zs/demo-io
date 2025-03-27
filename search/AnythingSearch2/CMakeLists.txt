cmake_minimum_required(VERSION 3.16)
project(FileSearchApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Concurrent)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/filelistmodel.cpp
    src/filelistmodel.h
    src/fileitemdelegate.cpp
    src/fileitemdelegate.h
    src/searchmanager.cpp
    src/searchmanager.h
    src/searchengine.h
    src/basicsearchengine.cpp
    src/basicsearchengine.h
    src/lucenesearchengine.cpp
    src/lucenesearchengine.h
    src/filedata.h
)

add_executable(FileSearchApp ${PROJECT_SOURCES})

target_link_libraries(FileSearchApp PRIVATE 
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Concurrent
    PkgConfig::Lucene
)

