#ifndef WORKER_H
#define WORKER_H

#include "connection.h"
#include <atomic>

// Worker进程类
class Worker {
public:
    Worker(int workerId, int listenFd, int maxConnections);
    ~Worker();

    // 设置共享的互斥锁地址（用于解决惊群问题）
    void setAcceptMutex(std::atomic<int>* acceptMutex);
    
    // 初始化Worker
    bool init();
    
    // Worker主循环
    void run();
    
    // 结束Worker
    void stop();
    
    // 处理新连接
    void handleNewConnection();

private:
    int workerId_;                     // Worker ID
    int listenFd_;                     // 监听套接字
    int maxConnections_;               // 最大连接数
    bool running_;                     // 运行标志
    ConnectionManager connManager_;    // 连接管理器
    std::atomic<int>* acceptMutex_;    // 共享的accept互斥锁
};

#endif // WORKER_H 