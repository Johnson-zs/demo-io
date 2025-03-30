#include "framework/worker_manager.h"
#include <QDebug>
#include <QUuid>
#include <QDirIterator>
#include <QCoreApplication>

namespace Framework {

WorkerManager::WorkerManager(QObject *parent)
    : QObject(parent)
    , m_server(new QLocalServer(this))
{
    // 设置服务器名称
    QString serverName = getUniqueServerName();
    
    // 如果已存在同名服务器，先移除
    m_server->removeServer(serverName);
    
    // 启动服务器
    if (!m_server->listen(serverName)) {
        qWarning() << "Failed to start local server:" << m_server->errorString();
    } else {
        qDebug() << "Local server started on" << serverName;
    }
    
    // 连接新连接信号
    connect(m_server, &QLocalServer::newConnection, this, &WorkerManager::handleNewConnection);
    
    // 设置默认Worker进程路径
    m_workerProcessPath = QCoreApplication::applicationDirPath() + "/worker";
}

WorkerManager::~WorkerManager()
{
    unloadAllWorkers();
    
    if (m_server->isListening()) {
        m_server->close();
    }
}

bool WorkerManager::loadWorker(const QString &pluginPath, RunMode mode)
{
    if (mode == RunMode::Thread) {
        return loadThreadWorker(pluginPath);
    } else {
        return startProcessWorker(pluginPath);
    }
}

int WorkerManager::loadWorkers(const QDir &directory, RunMode mode)
{
    if (!directory.exists()) {
        qWarning() << "Plugin directory does not exist:" << directory.absolutePath();
        return 0;
    }

    int count = 0;
    QDirIterator it(directory.absolutePath(), QDir::Files);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (QLibrary::isLibrary(filePath)) {
            if (loadWorker(filePath, mode)) {
                count++;
            }
        }
    }

    qInfo() << "Loaded" << count << "worker plugins from" << directory.absolutePath();
    return count;
}

void WorkerManager::unloadAllWorkers()
{
    // 关闭所有线程Worker
    for (auto it = m_threadWorkers.begin(); it != m_threadWorkers.end(); ++it) {
        ThreadWorkerInfo &info = it.value();
        
        // 终止Worker
        if (info.worker) {
            info.worker->terminate();
        }
        
        // 停止线程
        if (info.thread && info.thread->isRunning()) {
            info.thread->quit();
            info.thread->wait(1000);
        }
        
        // 卸载插件
        if (info.loader) {
            info.loader->unload();
            delete info.loader;
        }
        
        // 删除线程
        if (info.thread) {
            delete info.thread;
        }
    }
    m_threadWorkers.clear();
    
    // 关闭所有进程Worker
    for (auto it = m_processWorkers.begin(); it != m_processWorkers.end(); ++it) {
        ProcessWorkerInfo &info = it.value();
        
        // 关闭Socket
        if (info.socket && info.socket->isOpen()) {
            info.socket->close();
            delete info.socket;
        }
        
        // 终止进程
        if (info.process && info.process->state() != QProcess::NotRunning) {
            info.process->terminate();
            if (!info.process->waitForFinished(3000)) {
                info.process->kill();
            }
            delete info.process;
        }
    }
    m_processWorkers.clear();
}

bool WorkerManager::sendTask(const QString &workerId, const TaskMessage &task)
{
    // 检查Worker是否存在
    if (m_threadWorkers.contains(workerId)) {
        // 线程模式Worker
        WorkerBase *worker = m_threadWorkers[workerId].worker;
        if (worker) {
            return worker->processTask(task);
        }
    } else if (m_processWorkers.contains(workerId)) {
        // 进程模式Worker
        QLocalSocket *socket = m_processWorkers[workerId].socket;
        if (socket && socket->isOpen()) {
            // 序列化消息
            QByteArray data;
            QDataStream stream(&data, QIODevice::WriteOnly);
            stream << task;
            
            // 发送消息
            qint64 bytesWritten = socket->write(data);
            return bytesWritten == data.size();
        }
    }
    
    qWarning() << "Worker not found:" << workerId;
    return false;
}

QStringList WorkerManager::getWorkersForTaskType(const QString &taskType) const
{
    QStringList result;
    
    // 检查线程模式Worker
    for (auto it = m_threadWorkers.begin(); it != m_threadWorkers.end(); ++it) {
        if (it.value().worker && it.value().worker->capabilities().contains(taskType)) {
            result.append(it.key());
        }
    }
    
    // 检查进程模式Worker (我们需要发送特殊请求获取能力)
    // 这里简化处理，假设所有进程Worker都支持该任务类型
    for (auto it = m_processWorkers.begin(); it != m_processWorkers.end(); ++it) {
        result.append(it.key());
    }
    
    return result;
}

QStringList WorkerManager::getAllWorkerIds() const
{
    QStringList result;
    
    // 添加线程模式Worker
    for (auto it = m_threadWorkers.begin(); it != m_threadWorkers.end(); ++it) {
        result.append(it.key());
    }
    
    // 添加进程模式Worker
    for (auto it = m_processWorkers.begin(); it != m_processWorkers.end(); ++it) {
        result.append(it.key());
    }
    
    return result;
}

QStringList WorkerManager::getWorkerCapabilities(const QString &workerId) const
{
    // 检查线程模式Worker
    if (m_threadWorkers.contains(workerId) && m_threadWorkers[workerId].worker) {
        return m_threadWorkers[workerId].worker->capabilities();
    }
    
    // 进程模式Worker需要发送特殊请求获取能力
    // 这里简化处理，返回空列表
    return QStringList();
}

bool WorkerManager::hasWorker(const QString &workerId) const
{
    return m_threadWorkers.contains(workerId) || m_processWorkers.contains(workerId);
}

void WorkerManager::setWorkerProcessPath(const QString &path)
{
    m_workerProcessPath = path;
}

void WorkerManager::handleNewConnection()
{
    QLocalSocket *socket = m_server->nextPendingConnection();
    if (!socket) {
        return;
    }
    
    // 等待Worker发送注册消息
    connect(socket, &QLocalSocket::readyRead, this, &WorkerManager::handleProcessWorkerMessage);
}

void WorkerManager::handleProcessWorkerMessage()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) {
        return;
    }
    
    QByteArray data = socket->readAll();
    
    // 检查是否是注册消息
    QDataStream stream(data);
    int typeInt;
    stream >> typeInt;
    
    MessageType type = static_cast<MessageType>(typeInt);
    if (type == MessageType::REGISTER) {
        // 重置流位置
        stream.device()->seek(0);
        
        // 读取注册消息
        RegisterMessage reg;
        stream >> reg;
        
        // 添加到进程Worker列表
        ProcessWorkerInfo info;
        info.socket = socket;
        info.process = nullptr; // 这里不知道具体哪个进程，因为是由服务器接收的连接
        info.pluginPath = ""; // 同样未知
        
        m_processWorkers[reg.workerId] = info;
        
        // 设置socket关闭处理
        connect(socket, &QLocalSocket::disconnected, [this, reg]() {
            if (m_processWorkers.contains(reg.workerId)) {
                m_processWorkers.remove(reg.workerId);
                qDebug() << "Process worker disconnected:" << reg.workerId;
            }
        });
        
        qDebug() << "Process worker registered:" << reg.workerId
                 << "Capabilities:" << reg.capabilities;
        
        // 发出Worker加载成功信号
        emit workerLoaded(reg.workerId, RunMode::Process);
    } else {
        // 查找对应的Worker ID
        QString workerId;
        for (auto it = m_processWorkers.begin(); it != m_processWorkers.end(); ++it) {
            if (it.value().socket == socket) {
                workerId = it.key();
                break;
            }
        }
        
        if (!workerId.isEmpty()) {
            // 处理Worker消息
            handleWorkerMessage(workerId, data);
        } else {
            qWarning() << "Received message from unknown worker";
        }
    }
}

bool WorkerManager::loadThreadWorker(const QString &pluginPath)
{
    // 创建插件加载器
    QPluginLoader *loader = new QPluginLoader(pluginPath, this);
    
    if (!loader->load()) {
        qWarning() << "Failed to load plugin:" << loader->errorString();
        emit workerLoadFailed(pluginPath, loader->errorString());
        delete loader;
        return false;
    }
    
    // 获取插件实例
    QObject *instance = loader->instance();
    if (!instance) {
        qWarning() << "Failed to get plugin instance:" << loader->errorString();
        emit workerLoadFailed(pluginPath, "Failed to get plugin instance");
        loader->unload();
        delete loader;
        return false;
    }
    
    // 尝试转换为WorkerBase
    WorkerBase *worker = qobject_cast<WorkerBase*>(instance);
    if (!worker) {
        qWarning() << "Plugin is not a valid worker:" << pluginPath;
        emit workerLoadFailed(pluginPath, "Plugin is not a valid worker");
        loader->unload();
        delete loader;
        return false;
    }
    
    // 生成Worker ID
    QString workerId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    worker->setWorkerId(workerId);
    
    // 设置消息处理函数
    worker->setMessageHandler([this, workerId](const QByteArray &data) {
        handleWorkerMessage(workerId, data);
    });
    
    // 初始化Worker
    if (!worker->initialize()) {
        qWarning() << "Failed to initialize worker:" << pluginPath;
        emit workerLoadFailed(pluginPath, "Failed to initialize worker");
        loader->unload();
        delete loader;
        return false;
    }
    
    // 创建线程
    QThread *thread = new QThread(this);
    worker->moveToThread(thread);
    
    // 启动线程
    thread->start();
    
    // 保存Worker信息
    ThreadWorkerInfo info;
    info.thread = thread;
    info.worker = worker;
    info.loader = loader;
    
    m_threadWorkers[workerId] = info;
    
    qDebug() << "Thread worker loaded:" << workerId
             << "Capabilities:" << worker->capabilities();
    
    // 发出Worker加载成功信号
    emit workerLoaded(workerId, RunMode::Thread);
    
    return true;
}

bool WorkerManager::startProcessWorker(const QString &pluginPath)
{
    // 检查Worker进程可执行文件是否存在
    if (m_workerProcessPath.isEmpty() || !QFile::exists(m_workerProcessPath)) {
        qWarning() << "Worker process executable not found:" << m_workerProcessPath;
        emit workerLoadFailed(pluginPath, "Worker process executable not found");
        return false;
    }
    
    // 创建进程
    QProcess *process = new QProcess(this);
    
    // 设置进程参数
    QStringList args;
    args << "--plugin" << pluginPath
         << "--server" << m_server->serverName();
    
    // 启动进程
    process->start(m_workerProcessPath, args);
    
    // 检查进程是否启动成功
    if (!process->waitForStarted(5000)) {
        qWarning() << "Failed to start worker process:" << process->errorString();
        emit workerLoadFailed(pluginPath, "Failed to start worker process");
        delete process;
        return false;
    }
    
    // 注册进程结束处理
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitCode);
                Q_UNUSED(exitStatus);
                
                // 查找对应的Worker ID
                QString workerId;
                for (auto it = m_processWorkers.begin(); it != m_processWorkers.end(); ++it) {
                    if (it.value().process == process) {
                        workerId = it.key();
                        break;
                    }
                }
                
                if (!workerId.isEmpty()) {
                    qDebug() << "Process worker exited:" << workerId;
                    m_processWorkers.remove(workerId);
                }
                
                process->deleteLater();
            });
    
    // 注册进程错误处理
    connect(process, &QProcess::errorOccurred, [this, pluginPath, process](QProcess::ProcessError error) {
        qWarning() << "Worker process error:" << error << process->errorString();
        
        // 查找对应的Worker ID
        QString workerId;
        for (auto it = m_processWorkers.begin(); it != m_processWorkers.end(); ++it) {
            if (it.value().process == process) {
                workerId = it.key();
                break;
            }
        }
        
        if (!workerId.isEmpty()) {
            m_processWorkers.remove(workerId);
        }
        
        emit workerLoadFailed(pluginPath, "Worker process error: " + process->errorString());
        process->deleteLater();
    });
    
    // 此时，Worker还没有注册，它会在连接到服务器后发送注册消息
    // 我们先保存进程信息，在收到注册消息后再更新
    ProcessWorkerInfo info;
    info.process = process;
    info.socket = nullptr; // 连接后更新
    info.pluginPath = pluginPath;
    
    // 生成临时ID，正式ID会在注册时更新
    QString tempId = "temp_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_processWorkers[tempId] = info;
    
    return true;
}

void WorkerManager::handleWorkerMessage(const QString &workerId, const QByteArray &data)
{
    QDataStream stream(data);
    
    // 读取消息类型
    int typeInt;
    stream >> typeInt;
    MessageType type = static_cast<MessageType>(typeInt);
    
    // 重置流位置
    stream.device()->seek(0);
    
    // 根据消息类型处理
    switch (type) {
        case MessageType::TASK_STATUS: {
            TaskStatusMessage status;
            stream >> status;
            
            if (status.status == TaskStatusMessage::Status::RUNNING) {
                emit taskProgress(workerId, status.taskId, status.progress);
            } else if (status.status == TaskStatusMessage::Status::FAILED) {
                emit taskFailed(workerId, status.taskId, status.statusMessage);
            }
            break;
        }
        case MessageType::TASK_RESULT: {
            TaskResultMessage result;
            stream >> result;
            
            emit taskCompleted(workerId, result.taskId, result.success, result.result);
            break;
        }
        default:
            qWarning() << "Received unknown message type from worker:" << workerId << typeInt;
            break;
    }
}

QString WorkerManager::getUniqueServerName() const
{
    return "WorkerManagerServer_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace Framework 