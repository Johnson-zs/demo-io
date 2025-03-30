cmake_minimum_required(VERSION 3.16)
project(dfm-search VERSION 0.1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 启用自动处理 moc, uic, rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt 包
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Gui Widgets)

# 添加包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 设置库输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加子目录
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)

# 包导出配置
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/dfm-search-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# 安装包配置文件
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/dfm-search-config-version.cmake"
  DESTINATION lib/cmake/dfm-search
)

# 安装头文件
install(DIRECTORY include/ DESTINATION include) 
