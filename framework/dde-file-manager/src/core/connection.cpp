#include "connection.h"
#include <QLocalServer>
#include <QDataStream>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QUuid>
#include <QPromise>
#include <QDebug>

namespace DFM {

class Connection::Private {
public:
    Private(Connection *q) : q(q) {}
    
    Connection *q;
    QLocalSocket *socket = nullptr;
    bool ownsSocket = false;
    
    QByteArray readBuffer;
    QQueue<std::pair<MessageHeader, QByteArray>> sendQueue;
    QMutex sendMutex;
    
    // 读取消息状态
    int expectedDataSize = 0;
    MessageHeader currentHeader;
    bool readingHeader = true;
    
    bool processReceivedData();
};

Connection::Connection(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>(this))
{
}

Connection::Connection(QLocalSocket *socket, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>(this))
{
    setSocket(socket);
}

Connection::~Connection()
{
    close();
}

bool Connection::connectToServer(const QString &serverName, int timeout)
{
    if (d->socket) {
        close();
    }
    
    d->socket = new QLocalSocket(this);
    d->ownsSocket = true;
    
    connect(d->socket, &QLocalSocket::readyRead, this, &Connection::onReadyRead);
    connect(d->socket, &QLocalSocket::disconnected, this, &Connection::onDisconnected);
    connect(d->socket, static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::errorOccurred), 
           this, &Connection::onError);
           
    d->socket->connectToServer(serverName);
    if (!d->socket->waitForConnected(timeout)) {
        emit error(d->socket->errorString());
        close();
        return false;
    }
    
    emit connected();
    return true;
}

void Connection::setSocket(QLocalSocket *socket)
{
    if (d->socket) {
        close();
    }
    
    d->socket = socket;
    d->ownsSocket = false;
    
    if (socket) {
        connect(socket, &QLocalSocket::readyRead, this, &Connection::onReadyRead);
        connect(socket, &QLocalSocket::disconnected, this, &Connection::onDisconnected);
        connect(socket, static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::errorOccurred), 
               this, &Connection::onError);
               
        if (socket->isOpen()) {
            emit connected();
        }
    }
}

bool Connection::isConnected() const
{
    return d->socket && d->socket->isOpen() && 
           d->socket->state() == QLocalSocket::ConnectedState;
}

void Connection::close()
{
    if (d->socket) {
        d->socket->disconnect(this);
        
        if (d->ownsSocket) {
            d->socket->close();
            d->socket->deleteLater();
        }
        
        d->socket = nullptr;
    }
    
    d->sendQueue.clear();
    d->readBuffer.clear();
    d->expectedDataSize = 0;
    d->readingHeader = true;
    
    emit disconnected();
}

bool Connection::send(int cmd, const QByteArray &data)
{
    if (!isConnected()) {
        return false;
    }
    
    MessageHeader header;
    header.cmd = cmd;
    header.size = data.size();
    
    // 发送消息头和数据
    if (!sendHeader(header)) {
        return false;
    }
    
    if (data.size() > 0) {
        qint64 bytesWritten = d->socket->write(data);
        if (bytesWritten != data.size()) {
            emit error(tr("Failed to write complete data"));
            return false;
        }
    }
    
    return d->socket->waitForBytesWritten(5000);
}

bool Connection::waitForMessage(int timeout)
{
    if (!isConnected()) {
        return false;
    }
    
    return d->socket->waitForReadyRead(timeout);
}

bool Connection::sendHeader(const MessageHeader &header)
{
    if (!isConnected()) {
        return false;
    }
    
    // 序列化消息头
    QByteArray headerData;
    QDataStream stream(&headerData, QIODevice::WriteOnly);
    stream << header.cmd << header.size;
    
    // 发送消息头
    qint64 bytesWritten = d->socket->write(headerData);
    if (bytesWritten != headerData.size()) {
        emit error(tr("Failed to write complete header"));
        return false;
    }
    
    return true;
}

void Connection::onReadyRead()
{
    if (!d->socket) {
        return;
    }
    
    // 读取可用数据
    d->readBuffer.append(d->socket->readAll());
    
    // 处理缓冲区中的数据
    while (d->processReceivedData()) {
        // 继续处理下一个消息
    }
}

bool Connection::Private::processReceivedData()
{
    // 处理头部
    if (readingHeader) {
        if (readBuffer.size() < static_cast<int>(sizeof(int) * 2)) {
            // 头部数据不足
            return false;
        }
        
        // 读取头部
        QDataStream stream(readBuffer);
        stream >> currentHeader.cmd >> currentHeader.size;
        
        // 移除已读取的头部数据
        readBuffer.remove(0, static_cast<int>(sizeof(int) * 2));
        
        // 切换到读取数据模式
        readingHeader = false;
        expectedDataSize = currentHeader.size;
    }
    
    // 处理数据部分
    if (!readingHeader) {
        if (readBuffer.size() < expectedDataSize) {
            // 数据不足
            return false;
        }
        
        // 提取数据
        QByteArray messageData = readBuffer.left(expectedDataSize);
        readBuffer.remove(0, expectedDataSize);
        
        // 发送命令接收信号
        emit q->commandReceived(currentHeader.cmd, messageData);
        
        // 重置为读取头部模式
        readingHeader = true;
        expectedDataSize = 0;
        
        // 可能还有更多数据
        return !readBuffer.isEmpty();
    }
    
    return false;
}

void Connection::onDisconnected()
{
    emit disconnected();
}

void Connection::onError(QLocalSocket::LocalSocketError socketError)
{
    QString errorString;
    switch (socketError) {
        case QLocalSocket::ServerNotFoundError:
            errorString = tr("Server not found");
            break;
        case QLocalSocket::ConnectionRefusedError:
            errorString = tr("Connection refused");
            break;
        case QLocalSocket::PeerClosedError:
            errorString = tr("Remote closed the connection");
            break;
        default:
            errorString = d->socket->errorString();
            break;
    }
    
    emit error(errorString);
}

// ConnectionServer Implementation
class ConnectionServer::Private {
public:
    QLocalServer *server = nullptr;
    QString serverName;
};

ConnectionServer::ConnectionServer(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->server = new QLocalServer(this);
    connect(d->server, &QLocalServer::newConnection, this, &ConnectionServer::onNewConnection);
}

ConnectionServer::~ConnectionServer()
{
    close();
}

bool ConnectionServer::listen(const QString &name)
{
    close();
    
    // 如果没有提供名称，生成一个随机名称
    d->serverName = name.isEmpty() ? 
                   QStringLiteral("dfm-server-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)) : 
                   name;
    
    // 删除可能存在的旧socket
    QLocalServer::removeServer(d->serverName);
    
    if (!d->server->listen(d->serverName)) {
        emit error(d->server->errorString());
        return false;
    }
    
    return true;
}

void ConnectionServer::close()
{
    d->server->close();
}

QString ConnectionServer::serverName() const
{
    return d->serverName;
}

bool ConnectionServer::isListening() const
{
    return d->server->isListening();
}

void ConnectionServer::onNewConnection()
{
    while (d->server->hasPendingConnections()) {
        QLocalSocket *socket = d->server->nextPendingConnection();
        if (socket) {
            Connection *connection = new Connection(socket, this);
            emit newConnection(connection);
        }
    }
}

} // namespace DFM 