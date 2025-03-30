cmake_minimum_required(VERSION 3.16)
project(dfm-search VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt6包
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 添加库目标
add_subdirectory(src)

# 添加示例应用
add_subdirectory(examples)

# 安装配置
install(DIRECTORY include/dfm-search
        DESTINATION include
        COMPONENT Devel
        FILES_MATCHING PATTERN "*.h")

# 包配置
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/dfm-search-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/dfm-search-config-version.cmake"
    DESTINATION lib/cmake/dfm-search
    COMPONENT Devel
) 