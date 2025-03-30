#include "master.h"

#include <QDataStream>
#include <QDebug>
#include <QUuid>

Master::Master(QObject *parent) : QObject(parent) {
    // 注册消息类型
    
    server = new QLocalServer(this);
    healthCheckTimer = new QTimer(this);
    
    connect(server, &QLocalServer::newConnection, this, &Master::handleNewConnection);
    connect(healthCheckTimer, &QTimer::timeout, this, &Master::checkSlaveHealth);
    
    healthCheckTimer->start(5000); // 每5秒检查一次
}

Master::~Master() {
    // 清理所有slave进程
    for (const auto &slaveInfo : slaves) {
        if (slaveInfo.process) {
            slaveInfo.process->terminate();
            slaveInfo.process->waitForFinished();
            delete slaveInfo.process;
        }
        if (slaveInfo.socket) {
            slaveInfo.socket->disconnectFromServer();
            delete slaveInfo.socket;
        }
    }
}

void Master::start() {
    QLocalServer::removeServer("MasterSlaveDemo");
    if (server->listen("MasterSlaveDemo")) {
        qDebug() << "Master server started, listening for connections...";
        // 启动初始的slave进程
        for (int i = 0; i < 3; ++i) {
            startNewSlave();
        }
    } else {
        qDebug() << "Failed to start master server:" << server->errorString();
    }
}

void Master::startNewSlave() {
    QProcess *process = new QProcess(this);
    process->setProgram("slave");
    process->start();
    
    // 进程启动后，它会连接到我们的server，然后我们在handleNewConnection中处理
}

void Master::handleNewConnection() {
    QLocalSocket *slaveSocket = server->nextPendingConnection();
    connect(slaveSocket, &QLocalSocket::disconnected, this, &Master::handleSlaveDisconnected);
    connect(slaveSocket, &QLocalSocket::readyRead, this, &Master::handleSlaveMessage);
    
    qDebug() << "New slave connected";
}

void Master::submitTask(const TaskMessage &task) {
    // 先尝试找到最合适的slave
    QString slaveId = findBestSlaveForTask(task);
    
    qInfo() << "Submit task to salve: " << slaveId;
    if (slaveId.isEmpty()) {
        // 没有合适的slave，加入队列
        taskQueue.enqueue(task);
        qDebug() << "No available slave, task queued:" << task.taskId;
        return;
    }
    
    if (!assignTaskToSlave(task, slaveId)) {
        taskQueue.enqueue(task);
        qDebug() << "Failed to assign task to slave, queued:" << task.taskId;
    }
}

void Master::handleSlaveMessage() {
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    QDataStream stream(data);
    
    // 先只读取消息类型
    int typeInt;
    stream >> typeInt;
    MessageType type = static_cast<MessageType>(typeInt);
    
    // 重置流位置
    stream.device()->seek(0);
    
    // 根据消息类型直接反序列化为对应的派生类
    switch (type) {
        case MessageType::REGISTER: {
            RegisterMessage msg;
            stream >> msg;
            handleRegister(msg, socket); 
            break;
        }
        case MessageType::HEARTBEAT: {
            HeartbeatMessage msg;
            stream >> msg;
            handleHeartbeat(msg);
            break;
        }
        case MessageType::TASK_STATUS: {
            TaskStatusMessage msg;
            stream >> msg;
            handleTaskStatus(msg);
            break;
        }
        default:
            qDebug() << "Unknown message type received";
    }
}

void Master::handleSlaveDisconnected() {
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;
    
    // 找到对应的slave
    QString disconnectedSlaveId;
    for (auto it = slaves.begin(); it != slaves.end(); ++it) {
        if (it.value().socket == socket) {
            disconnectedSlaveId = it.key();
            break;
        }
    }
    
    if (!disconnectedSlaveId.isEmpty()) {
        markSlaveUnhealthy(disconnectedSlaveId);
        redistributeTasks(disconnectedSlaveId);
    }
    
    socket->deleteLater();
}

void Master::handleRegister(const RegisterMessage &msg, QLocalSocket *socket) {
    SlaveInfo slaveInfo;
    slaveInfo.slaveId = msg.slaveId;
    slaveInfo.hostname = msg.hostname;
    slaveInfo.cores = msg.cores;
    slaveInfo.totalMemory = msg.totalMemory;
    slaveInfo.capabilities = msg.capabilities;
    slaveInfo.socket = socket;
    slaveInfo.healthy = true;
    slaveInfo.lastHeartbeat = QDateTime::currentDateTime();
    
    slaves[msg.slaveId] = slaveInfo;
    
    qDebug() << "Slave registered:" << msg.slaveId << "on" << msg.hostname;
}

void Master::handleHeartbeat(const HeartbeatMessage &msg) {
    if (!slaves.contains(msg.slaveId)) return;
    
    auto &slave = slaves[msg.slaveId];
    slave.lastHeartbeat = QDateTime::currentDateTime();
    slave.runningTasks = msg.runningTaskIds;
    qDebug() << "Receive heart: " << slave.slaveId;
    // 如果之前不健康，现在恢复了，可以重新分配任务
    if (!slave.healthy) {
        slave.healthy = true;
        // 尝试分配队列中的任务
        while (!taskQueue.isEmpty() && slave.healthy) {
            TaskMessage task = taskQueue.dequeue();
            if (!assignTaskToSlave(task, msg.slaveId)) {
                taskQueue.enqueue(task);
                break;
            }
        }
    }
}

void Master::handleTaskStatus(const TaskStatusMessage &msg) {
    if (!tasks.contains(msg.taskId)) return;
    
    auto &taskInfo = tasks[msg.taskId];
    
    switch (msg.status) {
        case TaskStatusMessage::Status::COMPLETED:
            qDebug() << "Task completed:" << msg.taskId;
            tasks.remove(msg.taskId);
            break;
            
        case TaskStatusMessage::Status::FAILED:
            if (taskInfo.retryCount < taskInfo.maxRetries) {
                taskInfo.retryCount++;
                taskQueue.enqueue(taskInfo.originalTask);
                qDebug() << "Task failed, retrying:" << msg.taskId;
            } else {
                qDebug() << "Task failed permanently:" << msg.taskId;
                tasks.remove(msg.taskId);
            }
            break;
            
        default:
            break;
    }
}

void Master::checkSlaveHealth() {
    QDateTime now = QDateTime::currentDateTime();
    for (auto it = slaves.begin(); it != slaves.end(); ++it) {
        if (it.value().lastHeartbeat.secsTo(now) > 10) { // 10秒没有心跳就标记为不健康
            markSlaveUnhealthy(it.key());
        }
    }
}

void Master::markSlaveUnhealthy(const QString &slaveId) {
    if (!slaves.contains(slaveId)) return;
    
    auto &slave = slaves[slaveId];
    if (slave.healthy) {
        slave.healthy = false;
        redistributeTasks(slaveId);
    }
}

void Master::redistributeTasks(const QString &failedSlaveId) {
    // 将该slave上的所有任务重新加入队列
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        if (it.value().slaveId == failedSlaveId) {
            taskQueue.enqueue(it.value().originalTask);
        }
    }
}

bool Master::assignTaskToSlave(const TaskMessage &task, const QString &slaveId) {
    if (!slaves.contains(slaveId) || !slaves[slaveId].healthy) return false;
    
    auto &slave = slaves[slaveId];
    
    // 创建任务信息
    TaskInfo taskInfo;
    taskInfo.taskId = task.taskId;
    taskInfo.slaveId = slaveId;
    taskInfo.startTime = QDateTime::currentDateTime();
    taskInfo.deadline = task.deadline;
    taskInfo.retryCount = 0;
    taskInfo.maxRetries = 3;
    taskInfo.originalTask = task;
    
    // 发送任务给slave
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << task;
    
    qInfo() << "Assign task to slave: " << slave.slaveId << "Task " << task.taskId;
    slave.socket->write(data);
    tasks[task.taskId] = taskInfo;
    
    return true;
}

QString Master::findBestSlaveForTask(const TaskMessage &task) {
    // 简单的实现：找到第一个健康的、支持该任务类型的slave
    for (auto it = slaves.begin(); it != slaves.end(); ++it) {
        if (it.value().healthy && it.value().capabilities.contains(task.taskType)) {
            return it.key();
        }
    }
    return QString();
}

