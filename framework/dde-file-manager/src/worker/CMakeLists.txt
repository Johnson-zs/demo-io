set(EXEC_NAME dde-file-manager-worker)

# 使用 GLOB_RECURSE 递归搜索所有源文件
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# 创建可执行文件
add_executable(${EXEC_NAME} ${SOURCES})

# 链接库
target_link_libraries(${EXEC_NAME} PRIVATE
    dde-file-manager-core
    Qt6::Core
    Qt6::Network
    ${CMAKE_DL_LIBS}  # 用于动态加载 worker 插件
)

# 安装
install(TARGETS ${EXEC_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 