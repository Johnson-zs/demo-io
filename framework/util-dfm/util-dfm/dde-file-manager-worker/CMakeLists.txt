# 恢复 worker 程序的构建
set(WORKER_NAME dde-file-manager-worker)

set(WORKER_SOURCES
    main.cpp
)

# 添加可执行程序
add_executable(${WORKER_NAME}
    ${WORKER_SOURCES}
)

# 设置输出名称
set_target_properties(${WORKER_NAME} PROPERTIES
    OUTPUT_NAME ${WORKER_NAME}
)

# 链接库
target_link_libraries(${WORKER_NAME}
    PRIVATE
        dde-file-manager-core
        Qt6::Core
)

# 安装可执行程序
install(TARGETS ${WORKER_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 