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

// 定义信号，解决链接错误
void Worker::workerDied(Worker *) {}
void Worker::error(int, const QString &) {}
void Worker::disconnected() {}

Worker::Worker(const QString &proto, QObject *parent)
    : QObject(parent)
    , m_protocol(proto)
{
    qDebug() << "创建Worker实例, 协议:" << proto;
}

Worker::~Worker()
{
    qDebug() << "销毁Worker实例, 协议:" << m_protocol;
    
    // 关闭连接
    if (m_connection) {
        m_connection->close();
        delete m_connection;
    }
}

bool Worker::isAlive() const
{
    return m_connection && m_connection->isConnected();
}

bool Worker::isReady() const
{
    // 这里需要根据实际情况实现
    return !m_dead;
}

bool Worker::suspended()
{
    return m_connection && m_connection->suspended();
}

QString Worker::protocol() const
{
    return m_protocol;
}

void Worker::setProtocol(const QString &protocol)
{
    m_protocol = protocol;
}

QString Worker::workerProtocol() const
{
    return m_workerProtocol;
}

QString Worker::host() const
{
    return m_host;
}

quint16 Worker::port() const
{
    return m_port;
}

QString Worker::user() const
{
    return m_user;
}

QString Worker::passwd() const
{
    return m_passwd;
}

void Worker::setIdle()
{
    m_idleSince.restart();
}

void Worker::ref()
{
    m_refCount++;
}

void Worker::deref()
{
    m_refCount--;
}

bool Worker::isRefUsed() const
{
    return m_refCount > 0;
}

void Worker::aboutToDelete()
{
    // 可以在这里实现清理逻辑
}

void Worker::setWorkerThread(WorkerThread *thread)
{
    m_workerThread = thread;
}

int Worker::idleTime() const
{
    return m_idleSince.elapsed() / 1000;
}

void Worker::setPID(qint64 pid)
{
    m_pid = pid;
}

qint64 Worker::worker_pid() const
{
    return m_pid;
}

void Worker::setJob(SimpleJob *job)
{
    m_job = job;
}

SimpleJob *Worker::job() const
{
    return m_job;
}

void Worker::suspend()
{
    if (m_connection) {
        m_connection->suspend();
    }
}

void Worker::resume()
{
    if (m_connection) {
        m_connection->resume();
    }
}

void Worker::send(int cmd, const QByteArray &arr)
{
    if (m_connection) {
        m_connection->send(cmd, arr);
    }
}

void Worker::setHost(const QString &host, quint16 port, const QString &user, const QString &passwd)
{
    m_host = host;
    m_port = port;
    m_user = user;
    m_passwd = passwd;
}

void Worker::resetHost()
{
    m_host.clear();
    m_port = 0;
    m_user.clear();
    m_passwd.clear();
}

void Worker::setConfig(const QMap<QString, QString> &config)
{
    // 需要根据实际情况实现
}

void Worker::accept()
{
    // 需要根据实际情况实现
}

void Worker::timeout()
{
    // 需要根据实际情况实现
}

void Worker::gotInput()
{
    // 需要根据实际情况实现
}

void Worker::kill()
{
    // 需要根据实际情况实现
}

// static 方法
Worker* Worker::createWorker(const QString &protocol, const QUrl &url, int &error, QString &error_text)
{
    // 需要根据实际情况实现
    return new Worker(protocol);
}

} // namespace DFM 