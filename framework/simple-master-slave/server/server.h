#pragma once
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    ~Server();
    void sendMessage(QLocalSocket* socket, const QString& message);

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnected();

private:
    QLocalServer *server;
    QList<QLocalSocket*> clients;
    QHash<QLocalSocket*, qint32> expectedBytes;  // 每个连接的待接收字节数
}; 
