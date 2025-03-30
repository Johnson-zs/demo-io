#include "test_connection.h"
#include "../src/core/connection.h"

#include <QTest>
#include <QSignalSpy>
#include <QLocalServer>
#include <QLocalSocket>
#include <QUuid>

TestConnection::TestConnection(QObject *parent)
    : QObject(parent)
{
}

void TestConnection::initTestCase()
{
    // 创建服务器
    m_server = QSharedPointer<DFM::ConnectionServer>(new DFM::ConnectionServer());
    
    // 使用随机名称启动服务
    QString serverName = QString("test-connection-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    QVERIFY(m_server->listen(serverName));
    
    // 创建客户端连接
    m_connection = QSharedPointer<DFM::Connection>(new DFM::Connection());
}

void TestConnection::cleanupTestCase()
{
    // 关闭连接
    if (m_connection) {
        m_connection->close();
    }
    
    // 关闭服务器
    if (m_server) {
        m_server->close();
    }
}

void TestConnection::testConnectionCreation()
{
    QVERIFY(m_connection != nullptr);
    QVERIFY(!m_connection->isConnected());
}

void TestConnection::testServerListen()
{
    QVERIFY(m_server->isListening());
    QVERIFY(!m_server->serverName().isEmpty());
}

void TestConnection::testConnectToServer()
{
    if (m_connection->isConnected()) {
        m_connection->close();
    }
    
    // 监听连接信号
    QSignalSpy connectedSpy(m_connection.data(), &DFM::Connection::connected);
    
    // 连接到服务器
    bool connected = m_connection->connectToServer(m_server->serverName());
    QVERIFY(connected);
    
    // 验证信号是否触发
    QCOMPARE(connectedSpy.count(), 1);
    QVERIFY(m_connection->isConnected());
}

void TestConnection::testSendReceive()
{
    // 确保连接已建立
    if (!m_connection->isConnected()) {
        testConnectToServer();
    }
    
    // 创建一个新连接用于接收
    QSharedPointer<DFM::Connection> serverConnection;
    
    // 监听服务器的新连接信号
    QSignalSpy newConnectionSpy(m_server.data(), &DFM::ConnectionServer::newConnection);
    
    // 等待接收到新连接
    if (newConnectionSpy.count() < 1) {
        QVERIFY(newConnectionSpy.wait(5000));
    }
    
    // 获取服务器接收到的连接
    QList<QVariant> arguments = newConnectionSpy.takeFirst();
    DFM::Connection *receivedConnection = arguments.at(0).value<DFM::Connection*>();
    QVERIFY(receivedConnection != nullptr);
    
    // 监听命令接收信号
    QSignalSpy commandReceivedSpy(receivedConnection, &DFM::Connection::commandReceived);
    
    // 客户端发送命令
    int testCmd = 100;
    QByteArray testData = "Test Message";
    bool sent = m_connection->send(testCmd, testData);
    QVERIFY(sent);
    
    // 等待服务器接收到命令
    QVERIFY(commandReceivedSpy.wait(5000));
    QCOMPARE(commandReceivedSpy.count(), 1);
    
    // 验证接收到的命令和数据
    arguments = commandReceivedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), testCmd);
    QCOMPARE(arguments.at(1).toByteArray(), testData);
    
    // 测试从服务器向客户端发送响应
    QSignalSpy clientCommandReceivedSpy(m_connection.data(), &DFM::Connection::commandReceived);
    
    int responseCmd = 200;
    QByteArray responseData = "Response Data";
    QVERIFY(receivedConnection->send(responseCmd, responseData));
    
    // 等待客户端接收到响应
    QVERIFY(clientCommandReceivedSpy.wait(5000));
    QCOMPARE(clientCommandReceivedSpy.count(), 1);
    
    // 验证接收到的响应
    arguments = clientCommandReceivedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), responseCmd);
    QCOMPARE(arguments.at(1).toByteArray(), responseData);
    
    // 清理
    receivedConnection->deleteLater();
}

void TestConnection::testDisconnect()
{
    // 确保连接已建立
    if (!m_connection->isConnected()) {
        testConnectToServer();
    }
    
    // 监听断开连接信号
    QSignalSpy disconnectedSpy(m_connection.data(), &DFM::Connection::disconnected);
    
    // 断开连接
    m_connection->close();
    
    // 验证信号是否触发
    QCOMPARE(disconnectedSpy.count(), 1);
    QVERIFY(!m_connection->isConnected());
}

QTEST_MAIN(TestConnection) 