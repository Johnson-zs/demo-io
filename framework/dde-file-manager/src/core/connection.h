#pragma once

#include <QObject>
#include <QLocalSocket>
#include <QByteArray>
#include <memory>
#include <optional>
#include <future>

namespace DFM {

// 命令和消息类型定义
namespace Commands {
    // 系统命令 (1-99)
    constexpr int CMD_NONE = 0;
    constexpr int CMD_CONNECTED = 1;
    constexpr int CMD_DISCONNECT = 2;
    constexpr int CMD_QUIT = 3;
    
    // 任务命令 (100-199)
    constexpr int CMD_GET_DISK_USAGE = 100;
    constexpr int CMD_LIST_DIR = 101;
    constexpr int CMD_SEARCH = 102;
    
    // 响应消息 (200-299)
    constexpr int MSG_RESULT = 200;
    constexpr int MSG_DATA = 201;
    constexpr int MSG_ERROR = 202;
    constexpr int MSG_PROGRESS = 203;
}

// 消息头结构
struct MessageHeader {
    int cmd;
    int size;
};

// 用于应用和Worker间通信的连接类
class Connection : public QObject {
    Q_OBJECT
public:
    explicit Connection(QObject *parent = nullptr);
    explicit Connection(QLocalSocket *socket, QObject *parent = nullptr);
    ~Connection();

    // 连接方法
    bool connectToServer(const QString &serverName, int timeout = 30000);
    void setSocket(QLocalSocket *socket);
    bool isConnected() const;
    void close();

    // 发送和接收方法
    bool send(int cmd, const QByteArray &data = QByteArray());
    bool waitForMessage(int timeout = 30000);
    
    // 异步发送（使用C++17的std::future）
    template<typename T>
    std::future<T> sendAsync(int cmd, const QByteArray &data = QByteArray());

signals:
    void connected();
    void disconnected();
    void commandReceived(int cmd, const QByteArray &data);
    void error(const QString &errorString);

private slots:
    void onReadyRead();
    void onDisconnected();
    void onError(QLocalSocket::LocalSocketError error);

private:
    bool sendHeader(const MessageHeader &header);
    bool readMessage();
    
    class Private;
    std::unique_ptr<Private> d;
};

// 连接服务器类
class ConnectionServer : public QObject {
    Q_OBJECT
public:
    explicit ConnectionServer(QObject *parent = nullptr);
    ~ConnectionServer();

    // 启动和停止服务
    bool listen(const QString &name = QString());
    void close();
    
    // 服务器信息
    QString serverName() const;
    bool isListening() const;

signals:
    void newConnection(Connection *connection);
    void error(const QString &errorString);

private slots:
    void onNewConnection();

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace DFM 