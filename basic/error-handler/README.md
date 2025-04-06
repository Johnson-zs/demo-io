# C++23 错误处理示例

这个项目演示了如何使用C++23的`std::expected`和`std::error_category`机制进行错误处理。项目以"搜索"业务为例，展示了如何定义自定义错误类别和处理标准系统错误（`std::errc`）。

## 功能特性

- 使用C++23的`std::expected`返回结果或错误
- 自定义错误类别（`search_error_category`）
- 处理自定义错误和标准系统错误
- 演示多种错误情况和处理方式

## 项目结构

- `include/` - 头文件目录
  - `search_error.hpp` - 搜索错误定义
  - `search_service.hpp` - 搜索服务接口
- `src/` - 源文件目录
  - `search_error.cpp` - 搜索错误实现
  - `search_service.cpp` - 搜索服务实现
  - `main.cpp` - 主程序

## 构建要求

- CMake 3.20或更高版本
- 支持C++23的编译器
  - GCC 12+
  - Clang 15+

## 构建步骤

```bash
# 创建构建目录
mkdir build && cd build

# 配置
cmake ..

# 编译
cmake --build .

# 运行
./error_handler
```

## 示例输出

程序运行后，将展示多种搜索场景和可能的错误处理，包括：

1. 基本搜索功能（自定义错误）
2. 文件搜索功能（系统标准错误）
3. 索引重建（混合错误处理）
4. 配置服务（void返回值错误处理）

## 许可证

MIT 