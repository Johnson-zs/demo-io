#include "server.h"
#include <QDebug>

Server::Server(QObject *parent) : QObject(parent) {
    server = new QLocalServer(this);
    
    // 如果服务器已存在，先移除
    QLocalServer::removeServer("IPCServer");
    
    if (!server->listen("IPCServer")) {
        qDebug() << "Server failed to start. Error:" << server->errorString();
        return;
    }
    
    qDebug() << "Server is listening...";
    connect(server, &QLocalServer::newConnection, this, &Server::handleNewConnection);
}

Server::~Server() {
    for(QLocalSocket* socket : clients) {
        socket->disconnectFromServer();
    }
    qDeleteAll(clients);
}

void Server::handleNewConnection() {
    QLocalSocket *clientConnection = server->nextPendingConnection();
    connect(clientConnection, &QLocalSocket::readyRead, this, &Server::handleReadyRead);
    connect(clientConnection, &QLocalSocket::disconnected, this, &Server::handleDisconnected);
    
    clients.append(clientConnection);
    qDebug() << "New client connected";
}

void Server::handleReadyRead() {
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;
    
    while (socket->bytesAvailable() > 0) {
        if (expectedBytes[socket] == 0) {
            if (socket->bytesAvailable() < sizeof(qint32)) {
                return;
            }
            QDataStream stream(socket);
            stream >> expectedBytes[socket];
        }
        
        if (socket->bytesAvailable() < expectedBytes[socket]) {
            return;
        }
        
        QByteArray data = socket->read(expectedBytes[socket]);
        expectedBytes[socket] = 0;
        
        QString message = QString::fromUtf8(data);
        qDebug() << "Received:" << message;
        
        // 处理并回复消息
        QString upperCase = message.toUpper();
        sendMessage(socket, upperCase);
    }
}

void Server::sendMessage(QLocalSocket* socket, const QString& message) {
    QByteArray data = message.toUtf8();
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream << (qint32)data.size();
    frame.append(data);
    
    socket->write(frame);
}

void Server::handleDisconnected() {
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;
    
    clients.removeOne(socket);
    socket->deleteLater();
    qDebug() << "Client disconnected";
}
