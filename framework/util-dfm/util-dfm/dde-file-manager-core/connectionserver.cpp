#include "connectionserver.h"
#include "connection.h"
#include "connectionbackend.h"
#include <QDebug>

namespace DFM {

// 定义信号，解决链接错误
void ConnectionServer::newConnection() {}

ConnectionServer::ConnectionServer(QObject *parent)
    : QObject(parent)
    , backend(nullptr)
{
    qDebug() << "创建ConnectionServer实例";
}

ConnectionServer::~ConnectionServer()
{
    qDebug() << "销毁ConnectionServer实例";
    
    // 不需要删除backend，因为它是QObject的子对象
}

void ConnectionServer::listenForRemote()
{
    qDebug() << "ConnectionServer::listenForRemote()";
    
    // 如果已经在监听，则返回
    if (backend) {
        qWarning() << "ConnectionServer已经在监听";
        return;
    }
    
    // 创建连接后端
    backend = new ConnectionBackend(this);
    
    // 连接信号
    connect(backend, &ConnectionBackend::newConnection, 
            this, &ConnectionServer::newConnection);
    
    // 开始监听
    auto result = backend->listenForRemote();
    if (!result.success) {
        qWarning() << "ConnectionServer::listenForRemote(): 失败:" << result.error;
        delete backend;
        backend = nullptr;
    }
}

QUrl ConnectionServer::address() const
{
    if (backend) {
        return backend->address;
    }
    return QUrl();
}

bool ConnectionServer::isListening() const
{
    return backend && backend->state == ConnectionBackend::Listening;
}

void ConnectionServer::setNextPendingConnection(Connection *conn)
{
    // 这个方法在Connection实例化后调用
    qDebug() << "ConnectionServer::setNextPendingConnection()";
    
    if (!conn) {
        qWarning() << "ConnectionServer::setNextPendingConnection(): 连接为空";
        return;
    }
    
    // 可能需要将连接添加到队列中，或进行其他处理
}

} // namespace DFM 