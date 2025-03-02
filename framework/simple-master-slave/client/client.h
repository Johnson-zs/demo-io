#pragma once
#include <QLocalSocket>
#include <QObject>

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    ~Client();
    
    void sendMessage(const QString &message);

private slots:
    void handleConnected();
    void handleDisconnected();
    void handleReadyRead();
    void handleError(QLocalSocket::LocalSocketError error);

private:
    QLocalSocket *socket;
    qint32 expectedBytes;
}; 
