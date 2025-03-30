set(APP_NAME file-search-example)

# 查找Qt依赖
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Concurrent Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Widgets)

# 设置自动处理Qt相关文件
set(CMAKE_AUTOMOC ON)

# 源文件列表
set(SOURCES
    main.cpp
    mainwindow.cpp
    searchwidget.cpp
    resultmodel.cpp
)

# 头文件列表
set(HEADERS
    mainwindow.h
    searchwidget.h
    resultmodel.h
)

# 添加可执行文件
add_executable(${APP_NAME}
    ${SOURCES}
    ${HEADERS}
)

# 链接库
target_link_libraries(${APP_NAME} PRIVATE
    dfm-search
    Qt6::Core
    Qt6::Concurrent
    Qt6::Widgets
)

# 包含目录
target_include_directories(${APP_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# 安装目标
install(TARGETS ${APP_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 