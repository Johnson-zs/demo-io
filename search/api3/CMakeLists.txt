cmake_minimum_required(VERSION 3.16)
project(qsearch VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置库的版本信息
set(QSEARCH_VERSION_MAJOR 1)
set(QSEARCH_VERSION_MINOR 0)
set(QSEARCH_VERSION_PATCH 0)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)

# 设置库源文件
set(QSEARCH_HEADERS
    include/qsearch/global.h
    include/qsearch/search_query.h
    include/qsearch/search_result.h
    include/qsearch/search_options.h
    include/qsearch/search_manager.h
    include/qsearch/index_manager.h
    include/qsearch/engines/search_engine.h
    include/qsearch/engines/filename_engine.h
    include/qsearch/engines/fulltext_engine.h
    include/qsearch/engines/engine_factory.h
)

set(QSEARCH_SOURCES
    src/search_query.cpp
    src/search_result.cpp
    src/search_options.cpp
    src/search_manager.cpp
    src/index_manager.cpp
    src/engines/search_engine.cpp
    src/engines/filename_engine.cpp
    src/engines/fulltext_engine.cpp
    src/engines/engine_factory.cpp
    src/engines/filename/indexed_search.cpp
    src/engines/filename/traverse_search.cpp
    src/engines/fulltext/indexed_search.cpp
    src/engines/fulltext/direct_search.cpp
)

# 创建库
add_library(qsearch SHARED ${QSEARCH_SOURCES} ${QSEARCH_HEADERS})
target_compile_definitions(qsearch PRIVATE QSEARCH_LIBRARY)
target_include_directories(qsearch PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(qsearch PUBLIC
    Qt6::Core
    Qt6::Concurrent
)

# 设置安装路径
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/QSearch)

# 安装头文件
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 安装库
install(TARGETS qsearch
    EXPORT qsearch-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 导出目标
install(EXPORT qsearch-targets
    FILE QSearchTargets.cmake
    NAMESPACE QSearch::
    DESTINATION ${INSTALL_CONFIGDIR}
)

# 创建配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QSearchConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QSearchConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QSearchConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# 安装配置文件
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QSearchConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QSearchConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

# 构建示例
option(BUILD_EXAMPLES "Build example applications" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 生成pkg-config文件
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qsearch.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/qsearch.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qsearch.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig) 