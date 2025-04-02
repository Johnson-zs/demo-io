# 示例应用程序
add_executable(dfm6-search-example main.cpp)

target_link_libraries(dfm6-search-example
    PRIVATE
        dfm6-search
        Qt6::Core
)

# 安装示例应用
install(TARGETS dfm6-search-example
    RUNTIME DESTINATION bin
) 