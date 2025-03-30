#pragma once

#include <QObject>
#include <QSharedPointer>

namespace DFM {
class Connection;
class ConnectionServer;
}

class TestConnection : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testConnectionCreation();
    void testServerListen();
    void testConnectToServer();
    void testSendReceive();
    void testDisconnect();
    
private:
    QSharedPointer<DFM::ConnectionServer> m_server;
    QSharedPointer<DFM::Connection> m_connection;
}; 