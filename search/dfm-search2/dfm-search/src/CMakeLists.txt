set(LIB_NAME dfm-search)

set(SOURCES
    search_engine.cpp
    index_engine.cpp
    search_filter.cpp
    app_search.cpp
    ocr_search.cpp
    plugin_system.cpp
    filters/filename_filter.cpp
  #  filters/extension_filter.cpp
  #  filters/size_filter.cpp
  #  filters/date_filter.cpp
  #  filters/path_filter.cpp
  #  filters/filter_group.cpp
   # filters/custom_filter.cpp
    engines/realtime_filename_engine.cpp
    engines/indexed_filename_engine.cpp
    engines/realtime_content_engine.cpp
    engines/indexed_content_engine.cpp
    core/search_manager.cpp
    core/plugin_manager.cpp
    core/builtin_plugin_registry.cpp
    core/config_manager.cpp
    core/utils.cpp
)

set(HEADERS_INSTALL
    ../include/dfm-search/dfm_search.h
    ../include/dfm-search/search_engine.h
    ../include/dfm-search/index_engine.h
    ../include/dfm-search/search_filter.h
    ../include/dfm-search/app_search.h
    ../include/dfm-search/ocr_search.h
    ../include/dfm-search/plugin_system.h
)

# 自动对Q_OBJECT的类进行moc处理
set(CMAKE_AUTOMOC ON)

# 生成私有头文件目录
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include)

# 创建库
add_library(${LIB_NAME} SHARED ${SOURCES})

# 目标属性
set_target_properties(${LIB_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME DFMSearch
    PUBLIC_HEADER "${HEADERS_INSTALL}"
)

# 包含目录
target_include_directories(${LIB_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include
)

# 链接Qt库
target_link_libraries(${LIB_NAME}
    PUBLIC
        Qt6::Core
        Qt6::Concurrent
    PRIVATE
        # 此处可添加私有依赖
)

# 编译定义
target_compile_definitions(${LIB_NAME} PRIVATE
    QT_NO_KEYWORDS
    DFM_SEARCH_LIBRARY
)

# 安装目标
include(GNUInstallDirs)

install(TARGETS ${LIB_NAME}
    EXPORT ${LIB_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dfm-search
)

# 安装导出目标
install(EXPORT ${LIB_NAME}-targets
    FILE ${LIB_NAME}-targets.cmake
    NAMESPACE DFM::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
)

# 创建并安装配置文件
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
) 
