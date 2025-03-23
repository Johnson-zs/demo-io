#include "worker.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

Worker::Worker(int workerId, int listenFd, int maxConnections)
    : workerId_(workerId), 
      listenFd_(listenFd), 
      maxConnections_(maxConnections),
      running_(false),
      connManager_(maxConnections),
      acceptMutex_(nullptr) {
}

Worker::~Worker() {
    stop();
}

void Worker::setAcceptMutex(std::atomic<int>* acceptMutex) {
    acceptMutex_ = acceptMutex;
}

bool Worker::init() {
    // 初始化连接管理器
    if (!connManager_.init()) {
        std::cerr << "Worker " << workerId_ << ": 初始化连接管理器失败" << std::endl;
        return false;
    }
    
    // 添加监听套接字到epoll
    if (!connManager_.addListenFd(listenFd_)) {
        std::cerr << "Worker " << workerId_ << ": 添加监听套接字到epoll失败" << std::endl;
        return false;
    }
    
    std::cout << "Worker " << workerId_ << ": 初始化完成" << std::endl;
    return true;
}

void Worker::run() {
    running_ = true;
    
    std::cout << "Worker " << workerId_ << ": 开始运行 (PID: " << getpid() << ")" << std::endl;
    
    // 事件循环
    while (running_) {
        // 处理事件
        connManager_.processEvents(100); // 100ms超时
    }
    
    std::cout << "Worker " << workerId_ << ": 停止运行" << std::endl;
}

void Worker::stop() {
    running_ = false;
}

void Worker::handleNewConnection() {
    // 如果没有acceptMutex_，则直接处理连接
    if (!acceptMutex_) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientFd = accept(listenFd_, (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientFd >= 0) {
            connManager_.addConnection(clientFd);
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Worker " << workerId_ << ": accept失败: " << strerror(errno) << std::endl;
        }
        
        return;
    }
    
    // 使用原子变量实现简单的互斥锁，避免惊群问题
    int expected = 0;
    if (acceptMutex_->compare_exchange_strong(expected, 1)) {
        // 获取锁成功
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        while (running_) {
            int clientFd = accept(listenFd_, (struct sockaddr*)&clientAddr, &clientAddrLen);
            
            if (clientFd >= 0) {
                // 接受连接成功
                std::cout << "Worker " << workerId_ << ": 接受新连接: " << clientFd 
                          << " 来自 " << inet_ntoa(clientAddr.sin_addr) 
                          << ":" << ntohs(clientAddr.sin_port) << std::endl;
                
                connManager_.addConnection(clientFd);
            } else {
                // 没有更多连接或发生错误
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    std::cerr << "Worker " << workerId_ << ": accept失败: " << strerror(errno) << std::endl;
                }
                break;
            }
        }
        
        // 释放锁
        acceptMutex_->store(0);
    }
} 