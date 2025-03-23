# 设置源文件
set(SOURCES
    main.cpp
    master.cpp
    worker.cpp
    connection.cpp
    config.cpp
)

# 设置头文件
set(HEADERS
    master.h
    worker.h
    connection.h
    config.h
)

# 添加可执行文件
add_executable(mini_nginx ${SOURCES} ${HEADERS})

# 链接必要的库
target_link_libraries(mini_nginx pthread) 