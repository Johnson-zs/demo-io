#include "framework/worker_base.h"
#include <QDebug>
#include <QDataStream>
#include <QIODevice>

namespace Framework {

WorkerBase::WorkerBase(QObject *parent)
    : QObject(parent)
{
}

WorkerBase::~WorkerBase()
{
}

void WorkerBase::setWorkerId(const QString &id)
{
    m_workerId = id;
}

QString WorkerBase::workerId() const
{
    return m_workerId;
}

void WorkerBase::sendProgress(const QString &taskId, int percentage)
{
    if (!m_messageHandler) {
        qWarning() << "No message handler set for worker" << m_workerId;
        return;
    }

    // 创建进度消息
    TaskStatusMessage status;
    status.taskId = taskId;
    status.workerId = m_workerId;
    status.status = TaskStatusMessage::Status::RUNNING;
    status.progress = percentage;

    // 序列化消息
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << status;

    // 发送消息
    m_messageHandler(data);
}

void WorkerBase::finished(const QString &taskId, bool success, const QVariant &result)
{
    if (!m_messageHandler) {
        qWarning() << "No message handler set for worker" << m_workerId;
        return;
    }

    // 创建结果消息
    TaskResultMessage resultMsg;
    resultMsg.taskId = taskId;
    resultMsg.workerId = m_workerId;
    resultMsg.success = success;
    resultMsg.result = result;

    // 序列化消息
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << resultMsg;

    // 发送消息
    m_messageHandler(data);
}

void WorkerBase::error(const QString &taskId, int errorCode, const QString &errorMessage)
{
    if (!m_messageHandler) {
        qWarning() << "No message handler set for worker" << m_workerId;
        return;
    }

    // 创建错误消息
    TaskStatusMessage status;
    status.taskId = taskId;
    status.workerId = m_workerId;
    status.status = TaskStatusMessage::Status::FAILED;
    status.progress = 0;
    status.statusMessage = errorMessage;

    // 序列化消息
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << status;

    // 发送消息
    m_messageHandler(data);
}

void WorkerBase::warning(const QString &message)
{
    qWarning() << "Worker" << m_workerId << "warning:" << message;
}

bool WorkerBase::handleMessage(const QByteArray &data)
{
    try {
        QDataStream stream(data);
        
        // 读取消息类型
        int typeInt;
        stream >> typeInt;
        MessageType type = static_cast<MessageType>(typeInt);
        
        // 重置流位置
        stream.device()->seek(0);
        
        // 根据消息类型处理
        switch (type) {
            case MessageType::TASK_ASSIGN: {
                TaskMessage task;
                stream >> task;
                return processTask(task);
            }
            case MessageType::SHUTDOWN: {
                qInfo() << "Worker" << m_workerId << "received shutdown command";
                terminate();
                return true;
            }
            default: {
                qWarning() << "Worker" << m_workerId << "received unknown message type:" << typeInt;
                return false;
            }
        }
    } catch (const std::exception &e) {
        qWarning() << "Worker" << m_workerId << "failed to handle message:" << e.what();
        return false;
    }
}

void WorkerBase::setMessageHandler(std::function<void(const QByteArray &)> handler)
{
    m_messageHandler = handler;
}

} // namespace Framework 
