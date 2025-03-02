#include "slave.h"
#include "image_processor.h"

#include <QDataStream>
#include <QDebug>
#include <QHostInfo>
#include <QThread>
#include <QUuid>
#include <QCoreApplication>

Slave::Slave(QObject *parent)
    : QObject(parent)
{
    socket = new QLocalSocket(this);
    heartbeatTimer = new QTimer(this);
    slaveId = QUuid::createUuid().toString();

    // 连接信号槽
    connect(socket, &QLocalSocket::connected, this, &Slave::sendRegister);
    connect(socket, &QLocalSocket::disconnected, this, &Slave::reconnectToMaster);
    connect(socket, &QLocalSocket::errorOccurred, this, &Slave::handleConnectionError);
    connect(socket, &QLocalSocket::readyRead, this, &Slave::handleMessage);
    connect(heartbeatTimer, &QTimer::timeout, this, &Slave::sendHeartbeat);
}

Slave::~Slave()
{
    // 停止所有运行中的任务
    for (auto it = runningTasks.begin(); it != runningTasks.end(); ++it) {
        it.value()->stop();
        delete it.value();
    }

    if (socket->state() == QLocalSocket::ConnectedState) {
        socket->disconnectFromServer();
    }
}

void Slave::start()
{
    socket->connectToServer("MasterSlaveDemo");
    heartbeatTimer->start(3000);   // 每3秒发送一次心跳
}

void Slave::sendRegister()
{
    RegisterMessage msg;
    msg.slaveId = slaveId;
    msg.hostname = QHostInfo::localHostName();
    msg.cores = QThread::idealThreadCount();
    msg.totalMemory = 8589934592;   // 8GB，实际应该从系统获取
    msg.capabilities = { "ImageProcessing", "DataAnalysis" };   // 支持的任务类型

    // 测试序列化和反序列化
    QByteArray data;
    {
        QDataStream writeStream(&data, QIODevice::WriteOnly);
      //  writeStream.setVersion(QDataStream::Qt_5_15);
        writeStream << msg;
        
        qDebug() << "Original message:"
                 << "slaveId:" << msg.slaveId
                 << "hostname:" << msg.hostname;
    }

    // 从新的数据流读取
    {
        QDataStream readStream(data);
      //  readStream.setVersion(QDataStream::Qt_5_15);
        
        RegisterMessage uns;
        readStream >> uns;
        
        qDebug() << "Deserialized message:"
                 << "slaveId:" << uns.slaveId
                 << "hostname:" << uns.hostname;
    }

    // 实际发送数据
    socket->write(data);
    qDebug() << "Sent registration message: " << slaveId;
}

void Slave::sendHeartbeat()
{
    HeartbeatMessage msg;
    msg.slaveId = slaveId;
    collectResourceUsage(msg);

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << msg;

    socket->write(data);
}

void Slave::handleConnectionError(QLocalSocket::LocalSocketError error)
{
    qDebug() << "Connection error:" << error;
    QTimer::singleShot(5000, this, &Slave::reconnectToMaster);   // 5秒后重连
}

void Slave::handleMessage()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    QDataStream stream(data);
   // stream.setVersion(QDataStream::Qt_5_15);  // 设置数据流版本
    
    // 先只读取消息类型
    int typeInt;
    stream >> typeInt;
    MessageType type = static_cast<MessageType>(typeInt);

    // 重置流位置
    stream.device()->seek(0);
    
    // 根据消息类型读取完整消息
    switch (type) {
        case MessageType::TASK_ASSIGN: {
            TaskMessage task;
            stream >> task;
            handleTaskAssignment(task);
            break;
        }
        case MessageType::SHUTDOWN:
            qApp->quit();
            break;
        default:
            qDebug() << "Unknown message type received";
    }
}

void Slave::handleTaskAssignment(const TaskMessage &task)
{
    // 创建适当的任务执行器
    TaskExecutor *executor = nullptr;

    if (task.taskType == "ImageProcessing") {
        executor = new ImageProcessor(task, this);
    } else {
        sendTaskStatus(task.taskId, TaskStatusMessage::Status::FAILED,
                       0, "Unsupported task type");
        return;
    }

    // 连接信号
    connect(executor, &TaskExecutor::progressUpdated,
            this, &Slave::handleTaskProgress);
    connect(executor, &TaskExecutor::completed,
            this, &Slave::handleTaskCompletion);

    runningTasks[task.taskId] = executor;
    executor->start();

    sendTaskStatus(task.taskId, TaskStatusMessage::Status::ACCEPTED);
}

void Slave::handleTaskProgress(const QString &taskId, double progress)
{
    sendTaskStatus(taskId, TaskStatusMessage::Status::RUNNING, progress);
}

void Slave::handleTaskCompletion(const QString &taskId, bool success, const QVariant &result)
{
    TaskExecutor *executor = runningTasks.take(taskId);
    if (executor) {
        executor->deleteLater();
    }

    sendTaskStatus(taskId,
                   success ? TaskStatusMessage::Status::COMPLETED : TaskStatusMessage::Status::FAILED,
                   100,
                   result.toString());
}

void Slave::sendTaskStatus(const QString &taskId, TaskStatusMessage::Status status,
                           double progress, const QString &message)
{
    TaskStatusMessage msg;
    msg.taskId = taskId;
    msg.slaveId = slaveId;
    msg.status = status;
    msg.progress = progress;
    msg.statusMessage = message;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << msg;

    socket->write(data);
}

void Slave::reconnectToMaster()
{
    if (socket->state() != QLocalSocket::ConnectedState) {
        socket->connectToServer("MasterSlaveDemo");
    }
}

void Slave::collectResourceUsage(HeartbeatMessage &heartbeat)
{
    // 收集当前资源使用情况
    heartbeat.cpuUsage = 0.0;   // 实际应该从系统获取
    heartbeat.memoryUsage = 0;   // 实际应该从系统获取

    // 收集正在运行的任务ID
    heartbeat.runningTaskIds.clear();
    for (auto it = runningTasks.begin(); it != runningTasks.end(); ++it) {
        heartbeat.runningTaskIds.append(it.key());
    }
}
