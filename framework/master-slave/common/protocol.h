#pragma once
#include <QString>
#include <QDataStream>
#include <QDateTime>

// 消息类型枚举
enum class MessageType {
    // 控制消息
    REGISTER,           // Slave注册
    HEARTBEAT,         // 心跳
    SHUTDOWN,          // 关闭指令
    
    // 任务相关
    TASK_ASSIGN,       // 任务分配
    TASK_STATUS,       // 任务状态更新
    TASK_RESULT,       // 任务结果
    
    // 资源相关
    RESOURCE_USAGE,    // 资源使用情况
    
    // 错误处理
    ERROR_REPORT       // 错误报告
};

// 基础消息结构
struct Message {
    MessageType type;
    QString messageId;
    QDateTime timestamp;
    
    Message() : timestamp(QDateTime::currentDateTime()) {}
    
    virtual ~Message() = default;
    
    virtual void serialize(QDataStream &out) const {
        out << static_cast<int>(type) << messageId << timestamp;
    }
    
    virtual void deserialize(QDataStream &in) {
        int typeInt;
        in >> typeInt >> messageId >> timestamp;
        type = static_cast<MessageType>(typeInt);
    }
    
    friend QDataStream &operator<<(QDataStream &out, const Message &msg) {
        msg.serialize(out);
        return out;
    }
    
    friend QDataStream &operator>>(QDataStream &in, Message &msg) {
        msg.deserialize(in);
        return in;
    }
};

// Slave注册消息
struct RegisterMessage : public Message {
    QString slaveId;
    QString hostname;
    int cores;
    qint64 totalMemory;
    QStringList capabilities;
    
    RegisterMessage() {
        type = MessageType::REGISTER;
    }
    
    void serialize(QDataStream &out) const override {
        Message::serialize(out);
        out << slaveId << hostname << cores << totalMemory << capabilities;
    }
    
    void deserialize(QDataStream &in) override {
        Message::deserialize(in);
        in >> slaveId >> hostname >> cores >> totalMemory >> capabilities;
    }
    
    friend QDataStream &operator<<(QDataStream &out, const RegisterMessage &msg) {
        msg.serialize(out);
        return out;
    }
    
    friend QDataStream &operator>>(QDataStream &in, RegisterMessage &msg) {
        msg.deserialize(in);
        return in;
    }
};

// 心跳消息
struct HeartbeatMessage : public Message {
    QString slaveId;
    double cpuUsage;
    qint64 memoryUsage;
    QList<QString> runningTaskIds;
    
    HeartbeatMessage() {
        type = MessageType::HEARTBEAT;
    }
    
    friend QDataStream &operator<<(QDataStream &out, const HeartbeatMessage &msg) {
        out << static_cast<const Message&>(msg)
            << msg.slaveId << msg.cpuUsage << msg.memoryUsage 
            << msg.runningTaskIds;
        return out;
    }
    
    friend QDataStream &operator>>(QDataStream &in, HeartbeatMessage &msg) {
        in >> static_cast<Message&>(msg)
           >> msg.slaveId >> msg.cpuUsage >> msg.memoryUsage 
           >> msg.runningTaskIds;
        return in;
    }
};

// 任务消息
struct TaskMessage : public Message {
    QString taskId;
    QString slaveId;
    QString taskType;
    QVariantMap parameters;
    int priority;
    QDateTime deadline;
    
    TaskMessage() {
        type = MessageType::TASK_ASSIGN;
    }
    
    friend QDataStream &operator<<(QDataStream &out, const TaskMessage &msg) {
        out << static_cast<const Message&>(msg)
            << msg.taskId << msg.slaveId << msg.taskType 
            << msg.parameters << msg.priority << msg.deadline;
        return out;
    }
    
    friend QDataStream &operator>>(QDataStream &in, TaskMessage &msg) {
        in >> static_cast<Message&>(msg)
           >> msg.taskId >> msg.slaveId >> msg.taskType 
           >> msg.parameters >> msg.priority >> msg.deadline;
        return in;
    }
};

// 任务状态更新
struct TaskStatusMessage : public Message {
    QString taskId;
    QString slaveId;
    enum class Status {
        ACCEPTED,
        RUNNING,
        COMPLETED,
        FAILED,
        TIMEOUT
    } status;
    double progress;
    QString statusMessage;
    
    TaskStatusMessage() {
        type = MessageType::TASK_STATUS;
    }
    
    friend QDataStream &operator<<(QDataStream &out, const TaskStatusMessage &msg) {
        out << static_cast<const Message&>(msg)
            << msg.taskId << msg.slaveId 
            << static_cast<int>(msg.status)
            << msg.progress << msg.statusMessage;
        return out;
    }
    
    friend QDataStream &operator>>(QDataStream &in, TaskStatusMessage &msg) {
        int status;
        in >> static_cast<Message&>(msg)
           >> msg.taskId >> msg.slaveId 
           >> status
           >> msg.progress >> msg.statusMessage;
        msg.status = static_cast<TaskStatusMessage::Status>(status);
        return in;
    }
};


