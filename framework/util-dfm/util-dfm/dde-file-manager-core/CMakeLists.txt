set(CORE_LIBRARY_NAME dde-file-manager-core)

set(CORE_SOURCES
    connection.cpp
    connectionbackend.cpp
    connectionserver.cpp
    worker.cpp
    workerbase.cpp
    workerinterface.cpp
    scheduler.cpp
    commands.cpp
)

set(CORE_HEADERS
    connection.h
    connection_p.h
    connectionbackend.h
    connectionbackend_p.h
    connectionserver.h
    worker.h
    worker_p.h
    workerbase.h
    workerbase_p.h
    workerinterface.h
    workerinterface_p.h
    scheduler.h
    scheduler_p.h
    commands.h
)

# 添加库
add_library(${CORE_LIBRARY_NAME} SHARED
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

# 设置输出名称
set_target_properties(${CORE_LIBRARY_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME DFMCore
)

# 设置头文件安装路径
target_include_directories(${CORE_LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# 链接Qt库
target_link_libraries(${CORE_LIBRARY_NAME}
    PUBLIC
        Qt6::Core
        Qt6::Network
)

# 安装库和头文件
install(TARGETS ${CORE_LIBRARY_NAME}
    EXPORT DFMCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${CORE_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dfm-core
) 