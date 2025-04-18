cmake_minimum_required(VERSION 3.16)
project(util-dfm VERSION 0.1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 设置Qt依赖
find_package(Qt6 REQUIRED COMPONENTS Core Network)

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加子项目
add_subdirectory(dde-file-manager-core)
# 恢复被注释的子项目
add_subdirectory(dde-file-manager-worker)
add_subdirectory(dde-file-manager-workers)
add_subdirectory(tests)

# 安装配置
include(GNUInstallDirs)
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "*"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(DIRECTORY ${CMAKE_BINARY_DIR}/lib/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILES_MATCHING PATTERN "*.so*"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE) 