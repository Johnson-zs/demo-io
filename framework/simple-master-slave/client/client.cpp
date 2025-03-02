#include "client.h"
#include <QDebug>

Client::Client(QObject *parent) : QObject(parent) {
    socket = new QLocalSocket(this);
    
    connect(socket, &QLocalSocket::connected, this, &Client::handleConnected);
    connect(socket, &QLocalSocket::disconnected, this, &Client::handleDisconnected);
    connect(socket, &QLocalSocket::readyRead, this, &Client::handleReadyRead);
    connect(socket, static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::errorOccurred),
            this, &Client::handleError);
    
    socket->connectToServer("IPCServer");
}

Client::~Client() {
    if (socket->isOpen()) {
        socket->disconnectFromServer();
    }
}

void Client::sendMessage(const QString &message) {
    if (socket->state() == QLocalSocket::ConnectedState) {
        QByteArray data = message.toUtf8();
        // 消息格式：[size][data]
        QByteArray frame;
        QDataStream stream(&frame, QIODevice::WriteOnly);
        stream << (qint32)data.size();
        frame.append(data);
        
        socket->write(frame);
    } else {
        qDebug() << "Not connected to server";
    }
}

void Client::handleConnected() {
    qDebug() << "Connected to server";
}

void Client::handleDisconnected() {
    qDebug() << "Disconnected from server";
}

void Client::handleReadyRead() {
    while (socket->bytesAvailable() > 0) {
        if (expectedBytes == 0) {
            if (socket->bytesAvailable() < sizeof(qint32)) {
                return;  // 等待更多数据
            }
            QDataStream stream(socket);
            stream >> expectedBytes;
        }
        
        if (socket->bytesAvailable() < expectedBytes) {
            return;  // 等待更多数据
        }
        
        QByteArray data = socket->read(expectedBytes);
        expectedBytes = 0;
        
        QString message = QString::fromUtf8(data);
        qDebug() << "Received from server:" << message;
    }
}

void Client::handleError(QLocalSocket::LocalSocketError error) {
    qDebug() << "Error:" << error << socket->errorString();
}
