#ifndef MASTER_H
#define MASTER_H

#include "config.h"
#include <vector>
#include <atomic>
#include <csignal>

// Master进程类
class Master {
public:
    Master(const Config& config);
    ~Master();
    
    // 初始化Master
    bool init();
    
    // 启动Master
    bool start();
    
    // 停止Master
    void stop();
    
    // 等待Worker进程
    void waitForWorkers();
    
    // 重新加载配置
    bool reload();
    
    // 信号处理
    static void signalHandler(int sig);
    
    // 获取Master实例的全局指针（用于信号处理）
    static Master* getInstance() { return instance_; }
    static void setInstance(Master* instance) { instance_ = instance; }

private:
    // 创建监听套接字
    bool createListenSocket();
    
    // 设置套接字为非阻塞
    bool setNonBlocking(int sockfd);
    
    // 创建Worker进程
    bool createWorkers();
    
    // 创建单个Worker进程
    pid_t createWorker(int workerId);

private:
    static Master* instance_;          // 全局实例指针
    
    const Config& config_;             // 配置
    int listenFd_;                     // 监听套接字
    std::vector<pid_t> workerPids_;    // Worker进程的PID
    std::atomic<int>* acceptMutex_;    // 共享的accept互斥锁
    bool running_;                     // 运行标志
};

#endif // MASTER_H 