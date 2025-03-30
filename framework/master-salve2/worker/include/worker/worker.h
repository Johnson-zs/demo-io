#pragma once

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include <QMap>
#include <QUuid>

#include "framework/message.h"
#include "framework/common.h"
#include "worker/plugin_manager.h"

/**
 * @brief Worker类，负责与master通信并执行任务
 */
class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker();

    /**
     * @brief 启动Worker
     * @param pluginDir 插件目录
     * @param masterHost 主服务器地址
     * @param masterPort 主服务器端口
     */
    bool start(const QString &pluginDir, const QString &masterHost, int masterPort);

    /**
     * @brief 停止Worker
     */
    void stop();

private slots:
    /**
     * @brief 发送心跳
     */
    void sendHeartbeat();

    /**
     * @brief 处理连接错误
     */
    void handleConnectionError(QAbstractSocket::SocketError error);

    /**
     * @brief 处理收到的消息
     */
    void handleMessage();

    /**
     * @brief 处理重连
     */
    void reconnectToMaster();

    /**
     * @brief 处理任务进度更新
     */
    void handleTaskProgress(const QString &taskId, int progress);

    /**
     * @brief 处理任务完成
     */
    void handleTaskCompleted(const QString &taskId, bool success, const QVariant &result);

private:
    QTcpSocket *m_socket;
    QTimer *m_heartbeatTimer;
    QTimer *m_reconnectTimer;
    QString m_workerId;
    QString m_masterHost;
    int m_masterPort;
    PluginManager *m_pluginManager;
    QMap<QString, QString> m_runningTasks; // 任务ID -> 插件名称

    /**
     * @brief 发送注册信息
     */
    void sendRegister();

    /**
     * @brief 发送任务状态
     */
    void sendTaskStatus(const QString &taskId, Framework::TaskStatusMessage::Status status,
                        double progress = 0, const QString &message = QString());

    /**
     * @brief 处理任务分配
     */
    void handleTaskAssignment(const Framework::TaskMessage &task);

    /**
     * @brief 收集资源使用情况
     */
    void collectResourceUsage(Framework::HeartbeatMessage &heartbeat);
};