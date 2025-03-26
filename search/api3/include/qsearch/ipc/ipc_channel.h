#pragma once

#include <QObject>
#include <QVariant>
#include <QByteArray>
#include <QJsonObject>
#include "../global.h"

namespace QSearch {
namespace IPC {

class QSEARCH_EXPORT IPCChannel : public QObject {
    Q_OBJECT
public:
    enum ConnectionType {
        DBus,
        LocalSocket,
        SharedMemory
    };
    
    // 创建IPC通道
    static IPCChannel* create(ConnectionType type, const QString& channelId, QObject* parent = nullptr);
    
    // 连接到远程服务
    virtual bool connect() = 0;
    
    // 发送请求并等待响应 (同步)
    virtual QJsonObject sendRequest(const QString& method, const QJsonObject& params) = 0;
    
    // 发送请求 (异步)
    virtual void sendRequestAsync(const QString& method, const QJsonObject& params) = 0;
    
    // 发送通知 (无需响应)
    virtual void sendNotification(const QString& event, const QJsonObject& params) = 0;
    
signals:
    // 收到远程请求
    void requestReceived(const QString& method, const QJsonObject& params, QJsonObject& response);
    
    // 收到异步响应
    void responseReceived(const QString& requestId, const QJsonObject& response);
    
    // 收到远程通知
    void notificationReceived(const QString& event, const QJsonObject& params);
    
    // 连接状态变化
    void connectionStateChanged(bool connected);
    
protected:
    explicit IPCChannel(QObject* parent = nullptr);
};

} // namespace IPC
} // namespace QSearch 