set(PLUGIN_NAME dfm-du-worker)

# 使用 GLOB_RECURSE 递归搜索所有源文件
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# 创建共享库
add_library(${PLUGIN_NAME} MODULE ${SOURCES})

# 链接依赖库
target_link_libraries(${PLUGIN_NAME} PRIVATE
    dde-file-manager-core
    Qt6::Core
)

# 设置插件的输出路径
set_target_properties(${PLUGIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/dde-file-manager/workers"
)

# 安装
install(TARGETS ${PLUGIN_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dde-file-manager/workers
) 