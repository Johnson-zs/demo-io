#include "connection.h"
#include "connection_p.h"
#include "connectionbackend.h"
#include <QDebug>
#include <QMutexLocker>

namespace DFM {

// 定义信号，解决链接错误
void Connection::readyRead() {}
void Connection::disconnected() {}
void Connection::newConnection() {}

Connection::Connection(Type type, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<ConnectionPrivate>())
    , m_type(type)
{
    d->q = this;
    qDebug() << "创建Connection实例";
}

Connection::~Connection()
{
    qDebug() << "销毁Connection实例";
    // 不需要手动删除 d，std::unique_ptr 会自动管理释放
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

bool Connection::suspended() const
{
    return d->suspended;
}

void Connection::suspend()
{
    setSuspended(true);
}

void Connection::resume()
{
    setSuspended(false);
}

void Connection::connectToRemote(const QUrl &url)
{
    QMutexLocker locker(&d->mutex);
    
    // 如果已经初始化，则返回
    if (d->inited) {
        qWarning() << "Connection已经初始化";
        return;
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
}

void Connection::listenForRemote()
{
    QMutexLocker locker(&d->mutex);
    
    // 如果已经初始化，则返回
    if (d->inited) {
        qWarning() << "Connection已经初始化";
        return;
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
    }
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
    Connection *connection = new Connection(Type::Worker);
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

void Connection::close()
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

Task Connection::readCommand()
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

int Connection::read(int *_cmd, QByteArray &data)
{
    // 确保参数有效
    if (!_cmd) {
        return -1;
    }
    
    // 检查是否有可用任务
    if (!hasTaskAvailable()) {
        return -1;
    }
    
    // 读取任务
    Task task = readCommand();
    if (task.cmd == 0 && task.data.isEmpty()) {
        return -1;
    }
    
    // 设置输出参数
    *_cmd = task.cmd;
    data = task.data;
    
    return task.data.size();
}

void Connection::slotGotTask(const Task &task)
{
    qDebug() << "接收到命令:" << task.cmd << "数据大小:" << task.data.size();
    
    // 保存任务
    QMutexLocker locker(&d->mutex);
    d->incomingTasks.append(task);
    
    // 发出信号
    if (d->readMode == ReadMode::EventDriven) {
        d->signalEmitted = true;
        Q_EMIT readyRead();
    }
}

void Connection::slotBackendDisconnected()
{
    qDebug() << "连接断开";
    
    // 移除所有连接
    if (d->backend) {
        disconnect(d->backend, nullptr, this, nullptr);
    }
    
    // 标记为未初始化
    d->inited = false;
    
    // 发出信号
    Q_EMIT disconnected();
}

void Connection::slotNewConnection()
{
    qDebug() << "新的连接";
    
    // 发出信号
    Q_EMIT newConnection();
}

bool Connection::sendnow(int cmd, const QByteArray &data)
{
    qDebug() << "立即发送命令:" << cmd << "数据大小:" << data.size();
    
    // 如果没有初始化，则返回
    if (!d->inited || !d->backend) {
        qWarning() << "Connection未初始化";
        return false;
    }
    
    // 直接发送命令
    return d->backend->sendCommand(cmd, data);
}

void Connection::setReadMode(ReadMode readMode)
{
    d->readMode = readMode;
}

} // namespace DFM 