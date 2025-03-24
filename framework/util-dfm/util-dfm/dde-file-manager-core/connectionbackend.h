#ifndef DFM_CONNECTIONBACKEND_H
#define DFM_CONNECTIONBACKEND_H

#include <QObject>
#include <QUrl>
#include <memory>
#include <optional>
#include "connection_p.h"  // 包含 Task 的完整定义

class QLocalServer;
class QLocalSocket;

namespace DFM {

/**
 * @class ConnectionBackend
 * @brief 提供底层通信功能
 * 
 * ConnectionBackend类实现了底层的进程间通信机制，
 * 使用QLocalServer和QLocalSocket进行本地IPC通信。
 */
class ConnectionBackend : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 连接状态枚举
     */
    enum State {
        Idle,        ///< 空闲状态
        Connected,   ///< 已连接状态
        Listening    ///< 监听状态
    };
    
    /**
     * @brief 连接结果结构体
     */
    struct ConnectionResult {
        bool success;       ///< 是否成功
        QString error;      ///< 错误消息
    };
    
    /**
     * @brief 标准缓冲区大小
     */
    static constexpr int StandardBufferSize = 64 * 1024;

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ConnectionBackend(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~ConnectionBackend();

    /**
     * @brief 设置是否挂起
     * @param enable 是否启用挂起
     */
    void setSuspended(bool enable);
    
    /**
     * @brief 连接到远程端点
     * @param url 远程端点URL
     * @return 是否连接成功
     */
    bool connectToRemote(const QUrl &url);

    /**
     * @brief 监听远程连接
     * @return 监听结果
     */
    ConnectionResult listenForRemote();
    
    /**
     * @brief 等待接收任务
     * @param ms 超时时间(毫秒)
     * @return 是否成功接收
     */
    bool waitForIncomingTask(int ms);
    
    /**
     * @brief 发送命令
     * @param cmd 命令ID
     * @param data 命令数据
     * @return 是否发送成功
     */
    bool sendCommand(int cmd, const QByteArray &data) const;
    
    /**
     * @brief 获取下一个等待的连接
     * @return 新的连接后端
     */
    ConnectionBackend *nextPendingConnection();

Q_SIGNALS:
    /**
     * @brief 当接收到命令时发出的信号
     * @param task 任务数据
     */
    void commandReceived(const Task &task);
    
    /**
     * @brief 当连接断开时发出的信号
     */
    void disconnected();
    
    /**
     * @brief 当有新连接时发出的信号
     */
    void newConnection();

private Q_SLOTS:
    /**
     * @brief 处理套接字断开连接事件
     */
    void socketDisconnected();
    
    /**
     * @brief 处理套接字可读事件
     */
    void socketReadyRead();
    
    /**
     * @brief 处理新连接事件
     */
    void slotNewConnection();

public:
    State state;             ///< 当前状态
    QUrl address;            ///< 连接地址
    QString errorString;     ///< 错误信息
    QLocalServer *localServer; ///< 本地服务器
    QLocalSocket *socket;      ///< 套接字连接
    bool signalEmitted;        ///< 信号是否已发出
    std::optional<Task> pendingTask;  ///< 等待处理的任务
    
    static constexpr int HeaderSize = 10;  ///< 消息头大小
};

} // namespace DFM

#endif // DFM_CONNECTIONBACKEND_H 