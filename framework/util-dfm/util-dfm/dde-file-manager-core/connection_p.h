#ifndef DFM_CONNECTION_P_H
#define DFM_CONNECTION_P_H

#include "connection.h"
#include <QList>
#include <QMutex>

namespace DFM {

/**
 * @struct Task
 * @brief 表示一个通信任务
 */
struct Task {
    int cmd;          ///< 命令ID
    QByteArray data;  ///< 数据
    qint64 len;       ///< 数据长度
};

/**
 * @class ConnectionPrivate
 * @brief Connection类的私有实现
 */
class ConnectionPrivate {
public:
    ConnectionPrivate() 
        : backend(nullptr)
        , suspended(false)
        , readMode(Connection::ReadMode::EventDriven)
        , q(nullptr)
        , inited(false)
        , server(false)
        , signalEmitted(false)
    {}

    /**
     * @brief 设置后端连接
     * @param b 后端连接对象
     */
    void setBackend(ConnectionBackend *b);
    
    /**
     * @brief 从队列中取出任务并处理
     */
    void dequeue();
    
    /**
     * @brief 处理接收到的命令
     * @param task 任务数据
     */
    void commandReceived(const Task &task);
    
    /**
     * @brief 处理连接断开事件
     */
    void disconnected();

    ConnectionBackend *backend;         ///< 通信后端
    QList<Task> outgoingTasks;          ///< 发送任务队列
    QList<Task> incomingTasks;          ///< 接收任务队列
    bool suspended;                     ///< 是否已挂起
    Connection::ReadMode readMode;      ///< 读取模式
    Connection *q;                      ///< 指向拥有者的指针
    QMutex mutex;                       ///< 互斥锁
    bool inited;                        ///< 是否已初始化
    bool server;                        ///< 是否为服务器
    bool signalEmitted;                 ///< 信号是否已发出
};

} // namespace DFM

#endif // DFM_CONNECTION_P_H 