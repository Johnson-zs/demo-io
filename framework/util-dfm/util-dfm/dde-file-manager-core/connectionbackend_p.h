#ifndef DFM_CONNECTIONBACKEND_P_H
#define DFM_CONNECTIONBACKEND_P_H

#include <QObject>
#include <QUrl>
#include <QLocalServer>
#include <QLocalSocket>
#include <optional>

namespace DFM {

/**
 * @brief 连接后端私有类，处理底层通信
 */
class ConnectionBackend : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 连接状态
     */
    enum State {
        Idle,       ///< 空闲状态
        Listening,  ///< 正在监听连接
        Connected   ///< 已连接
    };

    /**
     * @brief 连接结果
     */
    struct ConnectionResult {
        bool success;     ///< 是否成功
        QString error;    ///< 错误信息
    };

    /**
     * @brief 任务结构体
     */
    struct Task {
        int cmd;          ///< 命令编号
        QByteArray data;  ///< 数据
        int len;          ///< 数据长度
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ConnectionBackend(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ConnectionBackend() override;

    /**
     * @brief 设置是否挂起
     * @param enable 是否启用
     */
    void setSuspended(bool enable);

    /**
     * @brief 连接到远程地址
     * @param url 远程地址
     * @return 是否成功
     */
    bool connectToRemote(const QUrl &url);

    /**
     * @brief 监听远程连接
     * @return 连接结果
     */
    ConnectionResult listenForRemote();

    /**
     * @brief 等待传入任务
     * @param ms 超时时间(毫秒)
     * @return 是否有任务
     */
    bool waitForIncomingTask(int ms);

    /**
     * @brief 发送命令
     * @param cmd 命令编号
     * @param data 数据
     * @return 是否成功
     */
    bool sendCommand(int cmd, const QByteArray &data) const;

    /**
     * @brief 获取下一个待处理的连接
     * @return 连接后端
     */
    ConnectionBackend *nextPendingConnection();

    /**
     * @brief 当前状态
     */
    State state;

    /**
     * @brief 地址
     */
    QUrl address;

    /**
     * @brief 本地服务器
     */
    QLocalServer *localServer;

    /**
     * @brief 套接字
     */
    QLocalSocket *socket;

    /**
     * @brief 错误字符串
     */
    QString errorString;

    /**
     * @brief 待处理的任务
     */
    std::optional<Task> pendingTask;

    /**
     * @brief 信号是否已发送
     */
    bool signalEmitted;

    /**
     * @brief 标准缓冲区大小
     */
    static constexpr int StandardBufferSize = 1024 * 8;

    /**
     * @brief 消息头大小
     */
    static constexpr int HeaderSize = 10;

Q_SIGNALS:
    /**
     * @brief 接收到命令信号
     * @param task 接收到的任务
     */
    void commandReceived(const Task &task);

    /**
     * @brief 断开连接信号
     */
    void disconnected();

    /**
     * @brief 新连接信号
     */
    void newConnection();

private Q_SLOTS:
    /**
     * @brief 套接字断开连接槽函数
     */
    void socketDisconnected();

    /**
     * @brief 套接字准备读取槽函数
     */
    void socketReadyRead();

    /**
     * @brief 新连接槽函数
     */
    void newConnection();
};

} // namespace DFM

#endif // DFM_CONNECTIONBACKEND_P_H 