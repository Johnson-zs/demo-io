#include "worker.h"
#include "worker_p.h"
#include "connection.h"
#include "connectionbackend.h"
#include "scheduler.h"

#include <QDebug>
#include <QFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>

namespace DFM {

Worker::Worker(const QString &proto, QObject *parent)
    : QObject(parent)
    , d(new WorkerPrivate(this))
{
    qDebug() << "创建Worker实例, 协议:" << proto;
    
    d->protocol = proto;
    d->state = WorkerPrivate::Idle;
}

Worker::~Worker()
{
    qDebug() << "销毁Worker实例, 协议:" << d->protocol;
    
    // 关闭连接
    if (d->connection) {
        d->connection->disconnect();
        delete d->connection;
    }
    
    delete d;
}

void Worker::setParent(QObject *parent)
{
    QObject::setParent(parent);
}

bool Worker::isAlive() const
{
    return d->state == WorkerPrivate::Running && d->connection && d->connection->isConnected();
}

bool Worker::isReady() const
{
    return d->state == WorkerPrivate::Idle || d->state == WorkerPrivate::Running;
}

bool Worker::isRefUsed() const
{
    return d->refCount > 0;
}

void Worker::hold()
{
    qDebug() << "Worker::hold() - 协议:" << d->protocol;
    d->onHold = true;
}

void Worker::unhold()
{
    qDebug() << "Worker::unhold() - 协议:" << d->protocol;
    d->onHold = false;
    
    // 尝试启动Worker
    if (d->state == WorkerPrivate::Idle) {
        d->startWorker();
    }
}

bool Worker::onHold() const
{
    return d->onHold;
}

int Worker::idleTime() const
{
    if (!d->idle) {
        return 0;
    }
    
    return d->idleStart.elapsed() / 1000;
}

QString Worker::protocol() const
{
    return d->protocol;
}

void Worker::suspend()
{
    qDebug() << "Worker::suspend() - 协议:" << d->protocol;
    
    // 挂起连接
    if (d->connection) {
        d->connection->suspend();
    }
}

void Worker::resume()
{
    qDebug() << "Worker::resume() - 协议:" << d->protocol;
    
    // 恢复连接
    if (d->connection) {
        d->connection->resume();
    }
}

void Worker::ref()
{
    d->refCount++;
    d->idle = false;
    d->idleStart = QTime();
}

void Worker::deref()
{
    d->refCount--;
    
    if (d->refCount <= 0) {
        d->refCount = 0;
        d->idle = true;
        d->idleStart.start();
    }
}

void Worker::connectWorker(Connection *connection)
{
    // 设置连接
    d->setConnection(connection);
    
    // 设置状态
    d->state = WorkerPrivate::Running;
    d->idle = true;
    d->idleStart.start();
    
    // 发送信号
    Q_EMIT workerConnected(this);
}

void Worker::disconnectWorker()
{
    qDebug() << "Worker::disconnectWorker() - 协议:" << d->protocol;
    
    // 断开连接
    if (d->connection) {
        d->connection->disconnect();
        delete d->connection;
        d->connection = nullptr;
    }
    
    // 设置状态
    d->state = WorkerPrivate::Idle;
    
    // 发送信号
    Q_EMIT workerDisconnected(this);
}

Connection *Worker::connection() const
{
    return d->connection;
}

WorkerPrivate::WorkerPrivate(Worker *parent)
    : q(parent)
    , connection(nullptr)
    , state(Idle)
    , onHold(false)
    , refCount(0)
    , idle(false)
{
}

WorkerPrivate::~WorkerPrivate()
{
}

void WorkerPrivate::setConnection(Connection *conn)
{
    // 如果已经有连接，则断开
    if (connection) {
        connection->disconnect();
        delete connection;
    }
    
    // 设置新连接
    connection = conn;
    
    // 连接信号
    QObject::connect(connection, &Connection::disconnected, q, &Worker::slotDisconnected);
}

void WorkerPrivate::startWorker()
{
    // 检查状态
    if (state != Idle || onHold) {
        return;
    }
    
    // 设置状态
    state = Launching;
    
    qDebug() << "启动Worker - 协议:" << protocol;
    
    // 创建临时服务器
    Connection connection;
    
    // 监听远程连接
    if (!connection.listenForRemote()) {
        qWarning() << "无法监听远程连接";
        state = Failed;
        return;
    }
    
    // 查找Worker程序
    QString exePath = QStandardPaths::findExecutable("dfm-" + protocol + "-worker");
    if (exePath.isEmpty()) {
        exePath = QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + 
                                 "/dfm-" + protocol + "-worker");
    }
    
    // 启动Worker进程
    QStringList args;
    args << connection.d->backend->address.toString();
    
    qDebug() << "启动Worker进程:" << exePath << args;
    
    // 启动Worker程序
    if (!QProcess::startDetached(exePath, args)) {
        qWarning() << "无法启动Worker进程:" << exePath;
        state = Failed;
        return;
    }
    
    // 等待连接
    if (!connection.waitForIncomingTask(10000)) { // 10秒超时
        qWarning() << "等待Worker连接超时";
        state = Failed;
        return;
    }
    
    // 接受连接
    Connection *newConnection = connection.nextPendingConnection();
    if (!newConnection) {
        qWarning() << "无法接受Worker连接";
        state = Failed;
        return;
    }
    
    // 设置连接
    setConnection(newConnection);
    
    // 设置状态
    state = Running;
    idle = true;
    idleStart.start();
    
    // 发送信号
    Q_EMIT q->workerConnected(q);
}

void Worker::slotDisconnected()
{
    qDebug() << "Worker::slotDisconnected() - 协议:" << d->protocol;
    
    // 设置状态
    d->state = WorkerPrivate::Idle;
    
    // 删除连接
    Connection *conn = d->connection;
    d->connection = nullptr;
    delete conn;
    
    // 发送信号
    Q_EMIT workerDisconnected(this);
    
    // 如果不在挂起状态且有引用，尝试重新启动
    if (!d->onHold && d->refCount > 0) {
        d->startWorker();
    }
}

} // namespace DFM 