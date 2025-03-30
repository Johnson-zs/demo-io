# KIO风格的Master-Worker框架

这是一个基于C++17和Qt的分布式任务处理框架，借鉴了KDE的KIO Worker架构设计。该框架支持在单独进程中运行Worker插件，实现了更好的资源隔离和稳定性。

## 架构概述

该框架的主要组件包括：

1. **框架核心库**：提供基础功能，包括消息通信协议、Worker基类和Worker管理器。
2. **Worker进程**：一个轻量级的进程壳，负责加载Worker插件并与主进程通信。
3. **Worker插件**：实现具体任务处理逻辑的插件，每个插件专注于特定类型的任务。
4. **工具程序**：包括测试客户端等辅助工具。

### 组件关系

```
主程序 <--[IPC通信]--> Worker进程(进程壳) <--[加载]--> Worker插件
```

## 架构特点

1. **进程隔离**：每个Worker在独立进程中运行，提高了系统稳定性
2. **插件化设计**：使用Qt插件机制，便于扩展和维护
3. **统一接口**：所有Worker遵循相同的接口，简化集成
4. **双层设计**：将元数据接口和实现代码分离，符合关注点分离原则

## 目录结构

```
├── framework/          # 框架核心
│   ├── include/        # 公共头文件
│   └── src/            # 框架实现
├── workers/            # Worker插件
│   ├── image_processor/  # 图像处理Worker
│   └── ...             # 其他Worker
└── tools/              # 辅助工具
    └── test_client/    # 测试客户端
```

## 构建要求

- CMake 3.14+
- C++17兼容的编译器
- Qt6或Qt5

## 构建步骤

```bash
# 创建构建目录
mkdir build && cd build

# 配置
cmake ..

# 编译
cmake --build .

# 安装（可选）
cmake --install .
```

## 使用示例

1. 启动测试客户端：

```bash
cd build
bin/test_client
```

测试客户端会自动启动Worker进程并加载图像处理插件，然后发送一个示例任务并显示任务执行过程。

## 开发新Worker

要开发新的Worker插件，需要以下步骤：

1. 创建一个继承自`WorkerFactory`的工厂类，用于提供Worker元数据和创建Worker实例
2. 创建一个继承自`WorkerBase`的Worker类，实现具体的任务处理逻辑
3. 为Worker插件创建JSON元数据文件，描述插件信息和支持的任务类型

参考`workers/image_processor`目录中的示例。

## 许可证

MIT

## 贡献

欢迎贡献代码、报告问题或提出改进建议。 