cmake_minimum_required(VERSION 3.16)
project(content-searcher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Debug information
message(STATUS "Building project ${PROJECT_NAME}")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
message(STATUS "Lucene++ Include dirs: ${Lucene_INCLUDE_DIRS}")
message(STATUS "Lucene++ Libraries: ${Lucene_LIBRARIES}")

find_package(Qt6 REQUIRED COMPONENTS Core)

# Add 3rdparty directory to include path
include_directories(src/3rdparty ${Lucene_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/contentsearcher.cpp
    src/3rdparty/chineseanalyzer.cpp
    src/3rdparty/chinesetokenizer.cpp
)

# Header files
set(HEADERS
    src/contentsearcher.h
    src/3rdparty/chineseanalyzer.h
    src/3rdparty/chinesetokenizer.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    PkgConfig::Lucene
)

# Install
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
) 
