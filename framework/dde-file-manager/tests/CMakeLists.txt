# 查找 Qt 测试组件
find_package(Qt6 REQUIRED COMPONENTS Test)

# 测试可执行文件名
set(TEST_NAME dfm-worker-framework-test)

# 使用 GLOB_RECURSE 递归搜索所有测试源文件
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE TEST_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# 创建测试可执行文件
add_executable(${TEST_NAME} ${TEST_SOURCES})

# 链接测试库
target_link_libraries(${TEST_NAME} PRIVATE
    dde-file-manager-core
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

# 添加测试
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME}) 