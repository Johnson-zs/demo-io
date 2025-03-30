cmake_minimum_required(VERSION 3.16)
project(dfm-search VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt相关设置
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Concurrent)
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)

# 添加子目录
add_subdirectory(src)
add_subdirectory(examples)

# 安装配置
include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 启用测试
option(ENABLE_TESTS "启用单元测试" OFF)
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 包配置
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/dfm-search-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/dfm-search-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dfm-search"
) 