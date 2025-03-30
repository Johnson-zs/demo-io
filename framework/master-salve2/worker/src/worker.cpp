#include "worker/worker.h"
#include <QDebug>
#include <QHostInfo>
#include <QThread>
#include <QCoreApplication>

Worker::Worker(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_workerId(Framework::Utils::generateUniqueId())
    , m_masterPort(Framework::Constants::DEFAULT_PORT)
    , m_pluginManager(new PluginManager(this))
{
    // 连接信号
    connect(m_socket, &QTcpSocket::readyRead, this, &Worker::handleMessage);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &Worker::handleConnectionError);
    
    connect(m_heartbeatTimer, &QTimer::timeout, this, &Worker::sendHeartbeat);
    connect(m_reconnectTimer, &QTimer::timeout, this, &Worker::reconnectToMaster);
    
    connect(m_pluginManager, &PluginManager::taskProgress, 
            this, &Worker::handleTaskProgress);
    connect(m_pluginManager, &PluginManager::taskCompleted, 
            this, &Worker::handleTaskCompleted);
}

Worker::~Worker()
{
    stop();
}

bool Worker::start(const QString &pluginDir, const QString &masterHost, int masterPort)
{
    qInfo() << "Starting worker with ID:" << m_workerId;
    
    // 加载插件
    if (!m_pluginManager->loadPlugins(pluginDir)) {
        qWarning() << "Failed to load any plugins from" << pluginDir;
        return false;
    }
    
    qInfo() << "Loaded plugins:" << m_pluginManager->getPluginNames().join(", ");
    qInfo() << "Supported task types:" << m_pluginManager->getSupportedTaskTypes().join(", ");
    
    // 设置主服务器信息
    m_masterHost = masterHost;
    m_masterPort = masterPort;
    
    // 连接到主服务器
    m_socket->connectToHost(m_masterHost, m_masterPort);
    
    if (m_socket->waitForConnected(Framework::Constants::CONNECTION_TIMEOUT)) {
        qInfo() << "Connected to master server at" << m_masterHost << ":" << m_masterPort;
        
        // 发送注册信息
        sendRegister();
        
        // 开始心跳
        m_heartbeatTimer->start(Framework::Constants::HEARTBEAT_INTERVAL);
        
        return true;
    } else {
        qWarning() << "Failed to connect to master server:" << m_socket->errorString();
        
        // 启动重连定时器
        m_reconnectTimer->start(5000);
        
        return false;
    }
}

void Worker::stop()
{
    qInfo() << "Stopping worker";
    
    // 停止定时器
    m_heartbeatTimer->stop();
    m_reconnectTimer->stop();
    
    // 关闭连接
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
    }
    
    // 卸载插件
    m_pluginManager->unloadPlugins();
}

void Worker::sendHeartbeat()
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    
    Framework::HeartbeatMessage heartbeat;
    heartbeat.workerId = m_workerId;
    
    // 收集资源使用情况
    collectResourceUsage(heartbeat);
    
    // 收集运行中的任务
    QStringList runningTaskIds;
    for (auto it = m_runningTasks.begin(); it != m_runningTasks.end(); ++it) {
        runningTaskIds.append(it.key());
    }
    heartbeat.runningTaskIds = runningTaskIds;
    
    // 发送心跳消息
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << heartbeat;
    
    m_socket->write(data);
}

void Worker::handleConnectionError(QAbstractSocket::SocketError error)
{
    qWarning() << "Socket error:" << error << "-" << m_socket->errorString();
    
    if (m_socket->state() != QAbstractSocket::ConnectedState && !m_reconnectTimer->isActive()) {
        m_reconnectTimer->start(5000);
    }
}

void Worker::handleMessage()
{
    QByteArray data = m_socket->readAll();
    QDataStream stream(data);
    
    // 读取消息类型
    int typeInt;
    stream >> typeInt;
    Framework::MessageType type = static_cast<Framework::MessageType>(typeInt);
    
    // 重置流位置
    stream.device()->seek(0);
    
    // 根据消息类型处理
    switch (type) {
        case Framework::MessageType::TASK_ASSIGN: {
            Framework::TaskMessage task;
            stream >> task;
            handleTaskAssignment(task);
            break;
        }
        case Framework::MessageType::SHUTDOWN: {
            qInfo() << "Received shutdown command";
            QCoreApplication::quit();
            break;
        }
        default: {
            qWarning() << "Received unknown message type:" << typeInt;
            break;
        }
    }
}

void Worker::reconnectToMaster()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_reconnectTimer->stop();
        return;
    }
    
    qInfo() << "Attempting to reconnect to master server...";
    m_socket->abort();
    m_socket->connectToHost(m_masterHost, m_masterPort);
    
    if (m_socket->waitForConnected(Framework::Constants::CONNECTION_TIMEOUT)) {
        qInfo() << "Reconnected to master server";
        sendRegister();
        m_heartbeatTimer->start(Framework::Constants::HEARTBEAT_INTERVAL);
        m_reconnectTimer->stop();
    } else {
        qWarning() << "Failed to reconnect:" << m_socket->errorString();
    }
}

void Worker::handleTaskProgress(const QString &taskId, int progress)
{
    sendTaskStatus(taskId, Framework::TaskStatusMessage::Status::RUNNING, progress);
}

void Worker::handleTaskCompleted(const QString &taskId, bool success, const QVariant &result)
{
    // 更新任务状态
    Framework::TaskStatusMessage::Status status = 
        success ? Framework::TaskStatusMessage::Status::COMPLETED 
                : Framework::TaskStatusMessage::Status::FAILED;
    
    sendTaskStatus(taskId, status, 100, success ? "Task completed" : "Task failed");
    
    // 发送结果消息
    Framework::TaskResultMessage resultMsg;
    resultMsg.taskId = taskId;
    resultMsg.workerId = m_workerId;
    resultMsg.success = success;
    resultMsg.result = result;
    
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << resultMsg;
    
    m_socket->write(data);
    
    // 从运行中任务列表移除
    m_runningTasks.remove(taskId);
}

void Worker::sendRegister()
{
    Framework::RegisterMessage reg;
    reg.workerId = m_workerId;
    reg.hostname = QHostInfo::localHostName();
    reg.cores = QThread::idealThreadCount();
    reg.totalMemory = Framework::Utils::getTotalMemory();
    
    // 添加能力
    reg.capabilities = m_pluginManager->getSupportedTaskTypes();
    
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << reg;
    
    m_socket->write(data);
    
    qInfo() << "Sent registration message to master server";
}

void Worker::sendTaskStatus(const QString &taskId, Framework::TaskStatusMessage::Status status,
                           double progress, const QString &message)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    
    Framework::TaskStatusMessage statusMsg;
    statusMsg.taskId = taskId;
    statusMsg.workerId = m_workerId;
    statusMsg.status = status;
    statusMsg.progress = progress;
    statusMsg.statusMessage = message;
    
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << statusMsg;
    
    m_socket->write(data);
}

void Worker::handleTaskAssignment(const Framework::TaskMessage &task)
{
    qInfo() << "Received task assignment:" << task.taskId << "Type:" << task.taskType;
    
    // 检查是否支持该任务类型
    if (!m_pluginManager->supportsTaskType(task.taskType)) {
        qWarning() << "Unsupported task type:" << task.taskType;
        sendTaskStatus(task.taskId, Framework::TaskStatusMessage::Status::FAILED, 
                       0, "Unsupported task type");
        return;
    }
    
    // 分配任务到插件
    if (m_pluginManager->executeTask(task.taskId, task.taskType, task.parameters)) {
        // 添加到运行中任务列表
        m_runningTasks[task.taskId] = task.taskType;
        
        // 发送接受状态
        sendTaskStatus(task.taskId, Framework::TaskStatusMessage::Status::ACCEPTED);
    } else {
        // 失败
        sendTaskStatus(task.taskId, Framework::TaskStatusMessage::Status::FAILED,
                      0, "Failed to assign task to plugin");
    }
}

void Worker::collectResourceUsage(Framework::HeartbeatMessage &heartbeat)
{
    heartbeat.cpuUsage = Framework::Utils::getCpuUsage();
    heartbeat.memoryUsage = Framework::Utils::getTotalMemory() - Framework::Utils::getAvailableMemory();
}
