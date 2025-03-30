#pragma once

#include <QString>
#include <QDataStream>
#include <QDateTime>
#include <QVariantMap>
#include <QStringList>

namespace Framework {

/**
 * @brief 消息类型枚举
 */
enum class MessageType {
    // 控制消息
    REGISTER,           // Worker注册
    HEARTBEAT,          // 心跳
    SHUTDOWN,           // 关闭指令
    
    // 任务相关
    TASK_ASSIGN,        // 任务分配
    TASK_STATUS,        // 任务状态更新
    TASK_RESULT,        // 任务结果
    
    // 资源相关
    RESOURCE_USAGE,     // 资源使用情况
    
    // 错误处理
    ERROR_REPORT        // 错误报告
};

/**
 * @brief 基础消息结构
 */
class Message {
public:
    MessageType type;
    QString messageId;
    QDateTime timestamp;
    
    Message();
    Message(MessageType type);
    virtual ~Message() = default;
    
    virtual void serialize(QDataStream &out) const;
    virtual void deserialize(QDataStream &in);
    
    friend QDataStream &operator<<(QDataStream &out, const Message &msg);
    friend QDataStream &operator>>(QDataStream &in, Message &msg);
};

/**
 * @brief Worker注册消息
 */
class RegisterMessage : public Message {
public:
    QString workerId;
    QString hostname;
    int cores;
    qint64 totalMemory;
    QStringList capabilities;
    
    RegisterMessage();
    
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;
};

/**
 * @brief 心跳消息
 */
class HeartbeatMessage : public Message {
public:
    QString workerId;
    double cpuUsage;
    qint64 memoryUsage;
    QList<QString> runningTaskIds;
    
    HeartbeatMessage();
    
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;
};

/**
 * @brief 任务消息
 */
class TaskMessage : public Message {
public:
    QString taskId;
    QString workerId;
    QString taskType;
    QVariantMap parameters;
    int priority;
    QDateTime deadline;
    
    TaskMessage();
    
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;
};

/**
 * @brief 任务状态更新
 */
class TaskStatusMessage : public Message {
public:
    QString taskId;
    QString workerId;
    
    enum class Status {
        ACCEPTED,
        RUNNING,
        COMPLETED,
        FAILED,
        TIMEOUT
    } status;
    
    double progress;
    QString statusMessage;
    
    TaskStatusMessage();
    
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;
};

/**
 * @brief 任务结果消息
 */
class TaskResultMessage : public Message {
public:
    QString taskId;
    QString workerId;
    bool success;
    QVariant result;
    
    TaskResultMessage();
    
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;
};

} // namespace Framework 