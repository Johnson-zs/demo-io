#include "connectionbackend.h"
#include "connectionbackend_p.h"
#include "connection_p.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QDebug>
#include <QUuid>

namespace DFM {

ConnectionBackend::ConnectionBackend(QObject *parent)
    : QObject(parent)
    , state(Idle)
    , localServer(nullptr)
    , socket(nullptr)
    , signalEmitted(false)
{
}

ConnectionBackend::~ConnectionBackend()
{
    // 关闭服务器
    if (localServer) {
        localServer->close();
        delete localServer;
        localServer = nullptr;
    }
    
    // 关闭套接字
    if (socket) {
        socket->close();
        delete socket;
        socket = nullptr;
    }
}

void ConnectionBackend::setSuspended(bool enable)
{
    if (socket) {
        // 设置套接字为非阻塞模式
        socket->setReadBufferSize(enable ? 0 : StandardBufferSize);
    }
}

bool ConnectionBackend::connectToRemote(const QUrl &url)
{
    if (state != Idle) {
        qWarning() << "ConnectionBackend: Already connected or listening";
        return false;
    }
    
    address = url;
    
    // 创建套接字
    socket = new QLocalSocket(this);
    
    // 连接信号
    connect(socket, &QLocalSocket::readyRead, this, &ConnectionBackend::socketReadyRead);
    connect(socket, &QLocalSocket::disconnected, this, &ConnectionBackend::socketDisconnected);
    
    // 连接到服务器
    socket->connectToServer(url.path());
    
    // 等待连接
    if (!socket->waitForConnected(5000)) {
        errorString = socket->errorString();
        qWarning() << "ConnectionBackend: Failed to connect to server:" << errorString;
        delete socket;
        socket = nullptr;
        return false;
    }
    
    // 设置状态
    state = Connected;
    
    return true;
}

ConnectionBackend::ConnectionResult ConnectionBackend::listenForRemote()
{
    if (state != Idle) {
        ConnectionResult result;
        result.success = false;
        result.error = "Already connected or listening";
        return result;
    }
    
    // 创建服务器
    localServer = new QLocalServer(this);
    
    // 生成唯一名称
    QString serverName = QUuid::createUuid().toString();
    serverName.remove('{').remove('}').remove('-');
    
    // 设置服务器名称
    if (!localServer->listen(serverName)) {
        errorString = localServer->errorString();
        qWarning() << "ConnectionBackend: Failed to start server:" << errorString;
        delete localServer;
        localServer = nullptr;
        
        ConnectionResult result;
        result.success = false;
        result.error = errorString;
        return result;
    }
    
    // 连接信号
    connect(localServer, &QLocalServer::newConnection, this, &ConnectionBackend::newConnection);
    
    // 设置状态
    state = Listening;
    address = QUrl("local:" + serverName);
    
    ConnectionResult result;
    result.success = true;
    return result;
}

bool ConnectionBackend::waitForIncomingTask(int ms)
{
    if (state != Connected || !socket) {
        return false;
    }
    
    // 如果已经有待处理的任务，直接返回
    if (pendingTask.has_value()) {
        signalEmitted = true;
        return true;
    }
    
    // 等待套接字可读
    if (!socket->waitForReadyRead(ms)) {
        return false;
    }
    
    // 如果有数据可读，处理它
    socketReadyRead();
    
    return pendingTask.has_value();
}

bool ConnectionBackend::sendCommand(int cmd, const QByteArray &data) const
{
    if (state != Connected || !socket) {
        return false;
    }
    
    // 构造消息头
    QByteArray header;
    QDataStream stream(&header, QIODevice::WriteOnly);
    stream << cmd;
    stream << data.size();
    
    // 发送消息头
    if (socket->write(header) != header.size()) {
        return false;
    }
    
    // 发送数据
    if (socket->write(data) != data.size()) {
        return false;
    }
    
    // 刷新缓冲区
    socket->flush();
    
    return true;
}

ConnectionBackend *ConnectionBackend::nextPendingConnection()
{
    if (state != Listening || !localServer) {
        return nullptr;
    }
    
    // 获取下一个连接
    QLocalSocket *socket = localServer->nextPendingConnection();
    if (!socket) {
        return nullptr;
    }
    
    // 创建新的后端
    ConnectionBackend *backend = new ConnectionBackend();
    backend->socket = socket;
    backend->state = Connected;
    
    // 转移所有权
    socket->setParent(backend);
    
    // 连接信号
    connect(socket, &QLocalSocket::readyRead, backend, &ConnectionBackend::socketReadyRead);
    connect(socket, &QLocalSocket::disconnected, backend, &ConnectionBackend::socketDisconnected);
    
    return backend;
}

void ConnectionBackend::socketDisconnected()
{
    // 发送断开连接信号
    Q_EMIT disconnected();
    
    // 设置状态
    state = Idle;
}

void ConnectionBackend::socketReadyRead()
{
    if (!socket) {
        return;
    }
    
    // 确保有足够的数据
    while (socket->bytesAvailable() >= HeaderSize) {
        // 读取消息头
        QByteArray header = socket->read(HeaderSize);
        QDataStream stream(header);
        
        int cmd;
        int size;
        
        stream >> cmd;
        stream >> size;
        
        // 确保有足够的数据
        if (socket->bytesAvailable() < size) {
            socket->ungetChar(header[header.size() - 1]);
            socket->ungetChar(header[header.size() - 2]);
            socket->ungetChar(header[header.size() - 3]);
            socket->ungetChar(header[header.size() - 4]);
            socket->ungetChar(header[header.size() - 5]);
            socket->ungetChar(header[header.size() - 6]);
            socket->ungetChar(header[header.size() - 7]);
            socket->ungetChar(header[header.size() - 8]);
            socket->ungetChar(header[header.size() - 9]);
            socket->ungetChar(header[header.size() - 10]);
            break;
        }
        
        // 读取数据
        QByteArray data = socket->read(size);
        
        // 创建任务
        Task task;
        task.cmd = cmd;
        task.data = data;
        task.len = size;
        
        // 设置待处理的任务
        pendingTask = task;
        
        // 发送信号
        if (!signalEmitted) {
            signalEmitted = true;
            Q_EMIT commandReceived(task);
        }
    }
}

void ConnectionBackend::newConnection()
{
    // 发送新连接信号
    Q_EMIT newConnection();
}

} // namespace DFM 