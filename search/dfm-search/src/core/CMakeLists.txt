set(LIB_NAME dfm-search)

FILE(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

# 创建库
add_library(${LIB_NAME} SHARED ${SOURCES})

# 链接 Qt 库
target_link_libraries(${LIB_NAME} PRIVATE
    Qt6::Core
    Qt6::Concurrent
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
)

# 设置头文件包含目录
target_include_directories(${LIB_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# 设置 C++ 特性
target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)

# 安装库
install(TARGETS ${LIB_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
) 
