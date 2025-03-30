cmake_minimum_required(VERSION 3.16)

# 设置示例应用项目
set(PROJECT_NAME dfm-search-example)
project(${PROJECT_NAME})

# 应用程序源文件
set(SOURCES
    main.cpp
    mainwindow.cpp
    searchwidget.cpp
)

# 应用程序头文件
set(HEADERS
    mainwindow.h
    searchwidget.h
)

# UI 文件
set(UI_FILES
    mainwindow.ui
    searchwidget.ui
)

# # 资源文件
# set(RESOURCES
#     resources.qrc
# )

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES})

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    dfm-search
)

# 安装目标
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
) 
