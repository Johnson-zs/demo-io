set(DU_WORKER_NAME dfm-du-worker)

set(DU_WORKER_SOURCES
    duworker.cpp
    workerfactory.cpp
)

set(DU_WORKER_HEADERS
    duworker.h
    workerfactory.h
)

# 添加库
add_library(${DU_WORKER_NAME} MODULE
    ${DU_WORKER_SOURCES}
    ${DU_WORKER_HEADERS}
)

# 设置属性
set_target_properties(${DU_WORKER_NAME} PROPERTIES
    PREFIX ""
)

# 链接库
target_link_libraries(${DU_WORKER_NAME}
    PRIVATE
        dde-file-manager-core
        Qt6::Core
)

# 安装库
install(TARGETS ${DU_WORKER_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dde-file-manager/workers
) 