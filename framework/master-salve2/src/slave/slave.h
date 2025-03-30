#pragma once
#include <QObject>
#include <QTimer>
#include <QLocalSocket>

#include "protocol.h"

class TaskExecutor;   // 前向声明

class Slave : public QObject
{
    Q_OBJECT
public:
    explicit Slave(QObject *parent = nullptr);
    ~Slave();

    void start();

private slots:
    void sendHeartbeat();
    void handleConnectionError(QLocalSocket::LocalSocketError error);
    void handleMessage();
    void handleTaskProgress(const QString &taskId, double progress);
    void handleTaskCompletion(const QString &taskId, bool success, const QVariant &result);

private:
    QLocalSocket *socket;
    QTimer *heartbeatTimer;
    QString slaveId;
    QHash<QString, TaskExecutor *> runningTasks;

    void sendRegister();
    void handleTaskAssignment(const TaskMessage &task);
    void sendTaskStatus(const QString &taskId, TaskStatusMessage::Status status,
                        double progress = 0, const QString &message = QString());
    void reconnectToMaster();
    void collectResourceUsage(HeartbeatMessage &heartbeat);
};

// 任务执行器基类
class TaskExecutor : public QObject
{
    Q_OBJECT
public:
    explicit TaskExecutor(const TaskMessage &task, QObject *parent = nullptr);
    virtual void start() = 0;
    virtual void stop() = 0;

signals:
    void progressUpdated(const QString &taskId, double progress);
    void completed(const QString &taskId, bool success, const QVariant &result);

protected:
    TaskMessage task;
    bool stopped;
};
