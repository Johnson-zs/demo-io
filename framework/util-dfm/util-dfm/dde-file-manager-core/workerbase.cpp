#include "workerbase.h"
#include "connection.h"
#include "connection_p.h"
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QTimer>

namespace DFM {

WorkerBase::WorkerBase(QObject *parent)
    : QObject(parent)
    , m_connection(nullptr)
    , m_suspended(false)
    , m_dead(false)
{
    qDebug() << "创建WorkerBase实例";
}

WorkerBase::~WorkerBase()
{
    qDebug() << "销毁WorkerBase实例";
    
    if (m_connection) {
        delete m_connection;
    }
}

void WorkerBase::setConnection(Connection *connection)
{
    if (m_connection) {
        disconnect(m_connection, &Connection::commandReceived, this, &WorkerBase::slotCommandReceived);
        delete m_connection;
    }
    
    m_connection = connection;
    
    if (m_connection) {
        connect(m_connection, &Connection::commandReceived, this, &WorkerBase::slotCommandReceived);
    }
}

void WorkerBase::dispatch()
{
    if (!m_connection) {
        qWarning() << "WorkerBase::dispatch(): 没有连接";
        return;
    }
    
    // 检查是否有等待的命令
    if (!m_connection->hasTaskAvailable()) {
        if (!m_connection->waitForIncomingTask(100)) { // 100ms超时
            return;
        }
    }
    
    // 读取命令
    Connection::Task task = m_connection->readCommand();
    if (task.cmd <= 0) {
        return;
    }
    
    // 处理命令
    QDataStream stream(task.data);
    int cmd = task.cmd;
    
    qDebug() << "WorkerBase::dispatch(): 接收到命令" << cmd;
    
    // 预定义命令处理
    switch (cmd) {
        case CMD_SUSPEND:
            suspend();
            break;
        case CMD_RESUME:
            resume();
            break;
        case CMD_QUIT:
            QCoreApplication::quit();
            break;
        case CMD_SPECIAL:
            special(task.data);
            break;
        default:
            // 派生类处理的命令
            dispatch(cmd, stream);
            break;
    }
}

void WorkerBase::suspend()
{
    qDebug() << "WorkerBase::suspend()";
    m_suspended = true;
    
    if (m_connection) {
        m_connection->suspend();
    }
}

void WorkerBase::resume()
{
    qDebug() << "WorkerBase::resume()";
    m_suspended = false;
    
    if (m_connection) {
        m_connection->resume();
    }
}

bool WorkerBase::suspended() const
{
    return m_suspended;
}

bool WorkerBase::connectToHost(const QUrl &host, const QString &user, const QString &pass)
{
    // 创建连接
    if (!m_connection) {
        m_connection = new Connection();
        connect(m_connection, &Connection::commandReceived, this, &WorkerBase::slotCommandReceived);
    }
    
    // 连接到主机
    return m_connection->connectToRemote(host);
}

void WorkerBase::send(int cmd, const QByteArray &data)
{
    if (!m_connection) {
        qWarning() << "WorkerBase::send(): 没有连接";
        return;
    }
    
    m_connection->send(cmd, data);
}

void WorkerBase::error(int errCode, const QString &errMsg)
{
    qDebug() << "WorkerBase::error():" << errCode << errMsg;
    
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    
    stream << errCode << errMsg;
    
    send(CMD_ERROR, data);
}

void WorkerBase::finished()
{
    qDebug() << "WorkerBase::finished()";
    send(CMD_FINISHED, QByteArray());
}

void WorkerBase::messageBox(const QString &text, const QString &title, 
                           const QString &primaryActionText, 
                           const QString &secondaryActionText)
{
    qDebug() << "WorkerBase::messageBox():" << title << text;
    
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    
    stream << text << title << primaryActionText << secondaryActionText;
    
    send(CMD_MESSAGEBOX, data);
}

void WorkerBase::slotCommandReceived(int cmd)
{
    qDebug() << "WorkerBase::slotCommandReceived():" << cmd;
    dispatch();
}

void WorkerBase::special(const QByteArray &data)
{
    // 默认实现，派生类可以重写
    QDataStream stream(data);
    QString command;
    stream >> command;
    
    qDebug() << "WorkerBase::special():" << command;
    
    // 子类应该重写此方法来处理特殊命令
    error(ERR_UNSUPPORTED_PROTOCOL, "特殊命令未实现: " + command);
}

} // namespace DFM 