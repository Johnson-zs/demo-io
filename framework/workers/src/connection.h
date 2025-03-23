#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <sys/epoll.h>

// 表示一个客户端连接
class Connection {
public:
    Connection(int sockfd);
    ~Connection();

    int getFd() const { return fd_; }
    bool isActive() const { return active_; }
    
    // 处理读事件
    bool handleRead();
    
    // 处理写事件
    bool handleWrite();
    
    // 关闭连接
    void close();

private:
    int fd_;            // 连接的文件描述符
    bool active_;       // 连接是否活跃
    std::string buffer_; // 读写缓冲区
};

// 连接管理类
class ConnectionManager {
public:
    ConnectionManager(int maxConnections);
    ~ConnectionManager();

    // 创建epoll实例
    bool init();
    
    // 添加监听socket到epoll
    bool addListenFd(int listenFd);
    
    // 添加客户端连接
    bool addConnection(int clientFd);
    
    // 处理事件
    void processEvents(int timeout = -1);
    
    // 关闭所有连接
    void closeAll();

private:
    int epollFd_;                      // epoll文件描述符
    int maxConnections_;               // 最大连接数
    struct epoll_event* events_;       // epoll事件数组
    Connection** connections_;         // 连接数组
};

#endif // CONNECTION_H 