#pragma once
#include <QObject>
#include <QTimer>
#include <QHash>
#include <QProcess>
#include <QLocalSocket>
#include <QLocalServer>
#include <QQueue>
#include <QDateTime>
#include <QStringList>

#include "protocol.h"

struct SlaveInfo
{
    QString slaveId;
    QString hostname;
    QDateTime lastHeartbeat;
    QProcess *process;
    QLocalSocket *socket;
    int cores;
    qint64 totalMemory;
    QStringList capabilities;
    QList<QString> runningTasks;
    bool healthy;
};

struct TaskInfo
{
    QString taskId;
    QString slaveId;
    QDateTime startTime;
    QDateTime deadline;
    int retryCount;
    int maxRetries;
    TaskMessage originalTask;
};

class Master : public QObject
{
    Q_OBJECT
public:
    explicit Master(QObject *parent = nullptr);
    ~Master();

    void start();
    void submitTask(const TaskMessage &task);

private slots:
    void checkSlaveHealth();
    void handleNewConnection();
    void handleSlaveDisconnected();
    void handleSlaveMessage();

private:
    QLocalServer *server;
    QTimer *healthCheckTimer;
    QHash<QString, SlaveInfo> slaves;
    QHash<QString, TaskInfo> tasks;
    QQueue<TaskMessage> taskQueue;

    void startNewSlave();
    void handleRegister(const RegisterMessage &msg, QLocalSocket *socket);
    void handleHeartbeat(const HeartbeatMessage &msg);
    void handleTaskStatus(const TaskStatusMessage &msg);
    void redistributeTasks(const QString &failedSlaveId);
    bool assignTaskToSlave(const TaskMessage &task, const QString &slaveId);
    QString findBestSlaveForTask(const TaskMessage &task);
    void markSlaveUnhealthy(const QString &slaveId);
};
