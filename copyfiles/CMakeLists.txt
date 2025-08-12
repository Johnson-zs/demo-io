cmake_minimum_required(VERSION 3.16)

project(CopyFilesDemo VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/ProgressDialog.cpp
    src/core/CopyAlgorithm.cpp
    src/core/CopyTask.cpp
    src/core/CopyWorker.cpp
    src/core/CopyTaskManager.cpp
    src/core/FileSystemUtils.cpp
    src/algorithms/DefaultCopyAlgorithm.cpp
    src/hooks/TaskHooks.cpp
)

# Header files
set(HEADERS
    src/ui/MainWindow.h
    src/ui/ProgressDialog.h
    src/core/CopyAlgorithm.h
    src/core/CopyTask.h
    src/core/CopyWorker.h
    src/core/CopyTaskManager.h
    src/core/ProgressObserver.h
    src/core/FileSystemUtils.h
    src/algorithms/DefaultCopyAlgorithm.h
    src/hooks/TaskHooks.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS}
    src/algorithms/synccopyalgorithm.h src/algorithms/synccopyalgorithm.cpp)

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Enable debug information in debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
endif()

# Create directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/hooks)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests) 
