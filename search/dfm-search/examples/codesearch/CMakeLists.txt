cmake_minimum_required(VERSION 3.16)

# 示例代码搜索应用
set(TARGET_NAME codesearch)

# 源文件
set(SOURCES
    main.cpp
    mainwindow.cpp
    searchwidget.cpp
)

# 头文件
set(HEADERS
    mainwindow.h
    searchwidget.h
)

# 创建可执行文件
add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS})

# 设置目标属性
set_target_properties(${TARGET_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC OFF
    AUTORCC OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 添加包含目录
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# 链接库
target_link_libraries(${TARGET_NAME} PRIVATE
    dfm-search
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

# 安装目标
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
) 