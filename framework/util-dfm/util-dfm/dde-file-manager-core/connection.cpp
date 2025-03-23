#include "connection.h"
#include "connection_p.h"
#include "connectionbackend.h"
#include <QDebug>
#include <QMutexLocker>

namespace DFM {

Connection::Connection(QObject *parent)
    : QObject(parent)
    , d(new ConnectionPrivate(this))
{
    qDebug() << "创建Connection实例";
}

Connection::~Connection()
{
    qDebug() << "销毁Connection实例";
    delete d;
}

void Connection::setSuspended(bool enable)
{
    qDebug() << "设置Connection挂起状态:" << enable;
    d->suspended = enable;
    
    if (d->backend) {
        d->backend->setSuspended(enable);
    }
}

bool Connection::isConnected() const
{
    return d->backend != nullptr;
}

bool Connection::inited() const
{
    return d->inited;
}

void Connection::suspend()
{
    setSuspended(true);
}

void Connection::resume()
{
    setSuspended(false);
}

bool Connection::connectToRemote(const QUrl &url)
{
    QMutexLocker locker(&d->mutex);
    
    // 如果已经初始化，则返回
    if (d->inited) {
        qWarning() << "Connection已经初始化";
        return false;
    }
    
    // 创建后端
    if (!d->backend) {
        d->backend = new ConnectionBackend(this);
    }
    
    // 连接信号
    connect(d->backend, &ConnectionBackend::commandReceived, this, &Connection::slotGotTask);
    connect(d->backend, &ConnectionBackend::disconnected, this, &Connection::slotBackendDisconnected);
    
    // 连接到远程
    bool result = d->backend->connectToRemote(url);
    if (result) {
        d->inited = true;
        d->server = false;
    }
    
    return result;
}

bool Connection::listenForRemote()
{
    QMutexLocker locker(&d->mutex);
    
    // 如果已经初始化，则返回
    if (d->inited) {
        qWarning() << "Connection已经初始化";
        return false;
    }
    
    // 创建后端
    if (!d->backend) {
        d->backend = new ConnectionBackend(this);
    }
    
    // 监听远程连接
    auto result = d->backend->listenForRemote();
    if (result.success) {
        d->inited = true;
        d->server = true;
        
        // 连接信号
        connect(d->backend, &ConnectionBackend::newConnection, this, &Connection::slotNewConnection);
        
        return true;
    }
    
    return false;
}

Connection *Connection::nextPendingConnection()
{
    QMutexLocker locker(&d->mutex);
    
    // 如果不是服务器，则返回
    if (!d->server || !d->backend) {
        return nullptr;
    }
    
    // 获取下一个连接
    ConnectionBackend *backend = d->backend->nextPendingConnection();
    if (!backend) {
        return nullptr;
    }
    
    // 创建新的连接
    Connection *connection = new Connection();
    connection->d->backend = backend;
    connection->d->inited = true;
    connection->d->server = false;
    
    // 连接信号
    connect(backend, &ConnectionBackend::commandReceived, connection, &Connection::slotGotTask);
    connect(backend, &ConnectionBackend::disconnected, connection, &Connection::slotBackendDisconnected);
    
    // 转移所有权
    backend->setParent(connection);
    
    return connection;
}

void Connection::disconnect()
{
    QMutexLocker locker(&d->mutex);
    
    // 删除后端
    if (d->backend) {
        delete d->backend;
        d->backend = nullptr;
    }
    
    // 重置状态
    d->inited = false;
    d->server = false;
}

bool Connection::send(int cmd, const QByteArray &data)
{
    qDebug() << "发送命令:" << cmd << "数据大小:" << data.size();
    
    // 如果没有初始化，则返回
    if (!d->inited || !d->backend) {
        qWarning() << "Connection未初始化";
        return false;
    }
    
    // 添加到任务队列
    Task task;
    task.cmd = cmd;
    task.data = data;
    task.len = data.size();
    
    d->outgoingTasks.append(task);
    
    // 处理任务队列
    processOutgoingTasks();
    
    return true;
}

void Connection::processOutgoingTasks()
{
    QMutexLocker locker(&d->mutex);
    
    // 如果没有待处理的任务，则返回
    if (d->outgoingTasks.isEmpty()) {
        return;
    }
    
    // 如果处于挂起状态，则返回
    if (d->suspended) {
        return;
    }
    
    // 处理所有任务
    while (!d->outgoingTasks.isEmpty()) {
        Task task = d->outgoingTasks.takeFirst();
        
        // 发送命令
        if (!d->backend->sendCommand(task.cmd, task.data)) {
            // 如果发送失败，则添加回队列
            d->outgoingTasks.prepend(task);
            break;
        }
    }
}

bool Connection::hasTaskAvailable() const
{
    QMutexLocker locker(&d->mutex);
    
    // 检查后端是否有待处理的任务
    if (d->backend) {
        return d->backend->pendingTask.has_value();
    }
    
    return false;
}

bool Connection::waitForIncomingTask(int ms)
{
    // 如果没有初始化，则返回
    if (!d->inited || !d->backend) {
        return false;
    }
    
    // 等待任务
    return d->backend->waitForIncomingTask(ms);
}

Connection::Task Connection::readCommand()
{
    QMutexLocker locker(&d->mutex);
    
    // 如果没有初始化，则返回
    if (!d->inited || !d->backend) {
        return Task();
    }
    
    // 获取待处理的任务
    if (d->backend->pendingTask.has_value()) {
        Task task = d->backend->pendingTask.value();
        d->backend->pendingTask.reset();
        d->backend->signalEmitted = false;
        return task;
    }
    
    return Task();
}

void Connection::slotGotTask(const Task &task)
{
    // 发送信号
    Q_EMIT commandReceived(task.cmd);
}

void Connection::slotBackendDisconnected()
{
    // 发送断开连接信号
    Q_EMIT disconnected();
}

void Connection::slotNewConnection()
{
    // 发送新连接信号
    Q_EMIT newConnection();
}

ConnectionPrivate::ConnectionPrivate(Connection *parent)
    : q(parent)
    , backend(nullptr)
    , inited(false)
    , server(false)
    , suspended(false)
{
}

ConnectionPrivate::~ConnectionPrivate()
{
    delete backend;
}

} // namespace DFM 