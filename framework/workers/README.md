# Mini-Nginx多Worker进程机制学习Demo

这个项目是一个简化的Nginx多Worker进程模型的示例实现，用于学习和理解Nginx的多Worker进程机制。

## 项目特点

- 采用Master-Worker多进程架构
- Master进程负责创建和管理Worker进程
- Worker进程使用事件驱动(epoll)处理客户端请求
- 多个Worker进程共享同一个监听套接字
- 使用原子变量实现简单的accept互斥锁，避免"惊群效应"
- 支持信号处理和优雅关闭

## 编译指南

### 依赖项

- CMake (3.10+)
- C++11支持的编译器
- Linux系统（项目使用了Linux特定的API，如epoll）

### 编译步骤

```bash
# 创建构建目录
mkdir build
cd build

# 运行CMake配置
cmake ..

# 编译
make
```

## 使用指南

### 运行

```bash
# 使用默认配置
./mini_nginx

# 使用指定的配置文件
./mini_nginx ../mini_nginx.conf
```

### 配置文件

配置文件是一个简单的键值对文件，示例如下：

```
# 监听端口
port=8080

# Worker进程数量
worker_processes=4

# 最大连接数
max_connections=1024
```

### 信号处理

程序响应以下信号：

- SIGINT/SIGTERM: 终止所有进程
- SIGHUP: 重新加载配置
- SIGCHLD: 自动处理子进程退出

## 测试

可以使用curl或浏览器访问服务器：

```bash
curl http://localhost:8080/
```

## 核心设计

### Master进程

- 负责解析配置
- 创建共享监听套接字
- 创建和管理Worker进程
- 处理信号

### Worker进程

- 使用epoll事件循环处理连接
- 处理客户端请求
- 使用互斥锁避免惊群效应

### 连接处理

- 非阻塞I/O
- 边缘触发(Edge-Triggered)模式
- 简单的HTTP响应

## 学习要点

通过这个demo，你可以学习：

1. 多进程模型如何工作
2. 如何在多个进程间共享套接字
3. 如何使用epoll处理大量连接
4. 如何避免惊群效应
5. 如何处理进程间通信
6. 如何处理信号和优雅关闭

## 限制

这个demo是为了教学目的而简化的，有以下限制：

- 没有实现完整的HTTP协议解析
- 没有实现配置热加载
- 没有实现日志系统
- 错误处理相对简单

## 与Nginx的区别

- Nginx有更完善的模块系统
- Nginx支持更多的事件驱动模型（epoll/kqueue/select等）
- Nginx有完整的HTTP/HTTPS/FastCGI等协议支持
- Nginx有更复杂的配置系统和更多优化 