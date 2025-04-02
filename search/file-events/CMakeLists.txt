cmake_minimum_required(VERSION 3.16)
project(file-monitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNL REQUIRED libnl-3.0 libnl-genl-3.0)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LIBNL_INCLUDE_DIRS}
)

set(SOURCES
    src/main.cpp
    src/filewatcher/filewatcher.cpp
    src/filewatcher/eventlistener.cpp
)

set(HEADERS
    src/filewatcher/filewatcher.h
    src/filewatcher/eventlistener.h
    src/utils/fsevents.h
    src/utils/logger.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    ${LIBNL_LIBRARIES}
)

# Install configuration
install(TARGETS ${PROJECT_NAME} DESTINATION bin) 