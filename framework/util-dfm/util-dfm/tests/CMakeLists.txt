set(TEST_DU_NAME test-du)

set(TEST_DU_SOURCES
    test-du.cpp
)

# 添加测试可执行程序
add_executable(${TEST_DU_NAME}
    ${TEST_DU_SOURCES}
)

# 链接库
target_link_libraries(${TEST_DU_NAME}
    PRIVATE
        dde-file-manager-core
        Qt6::Core
) 