# 示例应用程序
add_executable(dfm6-search-example main.cpp)
add_executable(dfm6-combined-search combined_search.cpp)
add_executable(dfm6-realtime-search realtime_search.cpp)
add_executable(dfm6-find-grep find_grep_example.cpp)
add_executable(dfm6-search-api-demo search_api_demo.cpp)

target_link_libraries(dfm6-search-example
    PRIVATE
        dfm6-search
        Qt6::Core
)

target_link_libraries(dfm6-combined-search
    PRIVATE
        dfm6-search
        Qt6::Core
)

target_link_libraries(dfm6-realtime-search
    PRIVATE
        dfm6-search
        Qt6::Core
)

target_link_libraries(dfm6-find-grep
    PRIVATE
        dfm6-search
        Qt6::Core
)

target_link_libraries(dfm6-search-api-demo
    PRIVATE
        dfm6-search
        Qt6::Core
)

# 安装示例应用
install(TARGETS 
    dfm6-search-example 
    dfm6-combined-search 
    dfm6-realtime-search 
    dfm6-find-grep
    dfm6-search-api-demo
    RUNTIME DESTINATION bin
) 