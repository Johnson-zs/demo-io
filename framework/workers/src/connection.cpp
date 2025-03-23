#include "connection.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Connection类实现
Connection::Connection(int sockfd) : fd_(sockfd), active_(true) {
    // 设置为非阻塞
    int flags = fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
}

Connection::~Connection() {
    if (active_) {
        close();
    }
}

bool Connection::handleRead() {
    char buf[4096];
    ssize_t n = ::read(fd_, buf, sizeof(buf) - 1);
    
    if (n > 0) {
        buf[n] = '\0';
        buffer_ += buf;
        
        // 简单的HTTP响应，实际应用中应该解析HTTP请求
        if (buffer_.find("\r\n\r\n") != std::string::npos) {
            return handleWrite();
        }
        return true;
    } else if (n == 0) {
        // 客户端关闭连接
        std::cout << "客户端关闭连接: " << fd_ << std::endl;
        close();
        return false;
    } else {
        // 读取错误
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "读取错误: " << strerror(errno) << std::endl;
            close();
            return false;
        }
        return true;
    }
}

bool Connection::handleWrite() {
    // 构造HTTP响应
    std::string response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain\r\n"
                          "Connection: close\r\n"
                          "Content-Length: 13\r\n"
                          "\r\n"
                          "Hello, World!";
    
    ssize_t n = ::write(fd_, response.c_str(), response.size());
    
    if (n > 0) {
        buffer_.clear();
        // 响应发送完毕，关闭连接
        close();
        return true;
    } else if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "写入错误: " << strerror(errno) << std::endl;
            close();
            return false;
        }
    }
    return true;
}

void Connection::close() {
    if (active_) {
        ::close(fd_);
        active_ = false;
        std::cout << "连接已关闭: " << fd_ << std::endl;
    }
}

// ConnectionManager类实现
ConnectionManager::ConnectionManager(int maxConnections) 
    : epollFd_(-1), maxConnections_(maxConnections), events_(nullptr), connections_(nullptr) {
}

ConnectionManager::~ConnectionManager() {
    closeAll();
    
    if (events_) {
        delete[] events_;
        events_ = nullptr;
    }
    
    if (connections_) {
        for (int i = 0; i < maxConnections_; ++i) {
            if (connections_[i]) {
                delete connections_[i];
            }
        }
        delete[] connections_;
        connections_ = nullptr;
    }
    
    if (epollFd_ >= 0) {
        ::close(epollFd_);
    }
}

bool ConnectionManager::init() {
    // 创建epoll实例
    epollFd_ = epoll_create1(0);
    if (epollFd_ < 0) {
        std::cerr << "创建epoll实例失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 分配事件数组和连接数组
    events_ = new epoll_event[maxConnections_];
    connections_ = new Connection*[maxConnections_];
    
    // 初始化连接数组
    for (int i = 0; i < maxConnections_; ++i) {
        connections_[i] = nullptr;
    }
    
    return true;
}

bool ConnectionManager::addListenFd(int listenFd) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenFd;
    
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, listenFd, &ev) < 0) {
        std::cerr << "添加监听套接字到epoll失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

bool ConnectionManager::addConnection(int clientFd) {
    // 确保文件描述符在有效范围内
    if (clientFd >= maxConnections_) {
        std::cerr << "连接文件描述符超出范围: " << clientFd << std::endl;
        ::close(clientFd);
        return false;
    }
    
    // 创建新连接
    connections_[clientFd] = new Connection(clientFd);
    
    // 添加到epoll
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // 边缘触发模式
    ev.data.fd = clientFd;
    
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, clientFd, &ev) < 0) {
        std::cerr << "添加客户端连接到epoll失败: " << strerror(errno) << std::endl;
        delete connections_[clientFd];
        connections_[clientFd] = nullptr;
        return false;
    }
    
    std::cout << "新客户端连接: " << clientFd << std::endl;
    return true;
}

void ConnectionManager::processEvents(int timeout) {
    int numEvents = epoll_wait(epollFd_, events_, maxConnections_, timeout);
    
    if (numEvents < 0) {
        if (errno != EINTR) {
            std::cerr << "epoll_wait错误: " << strerror(errno) << std::endl;
        }
        return;
    }
    
    for (int i = 0; i < numEvents; ++i) {
        int fd = events_[i].data.fd;
        
        // 处理连接
        if (fd >= 0 && fd < maxConnections_ && connections_[fd]) {
            if (events_[i].events & EPOLLIN) {
                connections_[fd]->handleRead();
            }
            
            if (events_[i].events & EPOLLOUT) {
                connections_[fd]->handleWrite();
            }
            
            // 处理错误情况
            if (events_[i].events & (EPOLLERR | EPOLLHUP)) {
                std::cerr << "连接错误: " << fd << std::endl;
                delete connections_[fd];
                connections_[fd] = nullptr;
            }
        }
    }
}

void ConnectionManager::closeAll() {
    if (connections_) {
        for (int i = 0; i < maxConnections_; ++i) {
            if (connections_[i]) {
                delete connections_[i];
                connections_[i] = nullptr;
            }
        }
    }
} 