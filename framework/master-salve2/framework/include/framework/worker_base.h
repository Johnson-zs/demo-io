#pragma once

#include <QObject>
#include <QVariant>
#include <QUrl>
#include <functional>

#include "message.h"

namespace Framework {

/**
 * @brief Worker基类，负责处理具体任务
 * 
 * Worker基类专注于任务处理逻辑，不关心底层通信细节。
 * 通信由WorkerProcess处理，Worker只需处理任务并报告进度和结果。
 */
class WorkerBase : public QObject {
    Q_OBJECT
public:
    explicit WorkerBase(QObject *parent = nullptr);
    virtual ~WorkerBase();

    /**
     * @brief 设置Worker的标识符
     * @param id Worker标识符
     */
    void setWorkerId(const QString &id);

    /**
     * @brief 获取Worker的标识符
     * @return Worker标识符
     */
    QString workerId() const;

    /**
     * @brief 发送任务进度更新
     * @param taskId 任务ID
     * @param percentage 进度百分比(0-100)
     */
    void sendProgress(const QString &taskId, int percentage);

    /**
     * @brief 完成任务并发送结果
     * @param taskId 任务ID
     * @param success 是否成功
     * @param result 任务结果
     */
    void finished(const QString &taskId, bool success, const QVariant &result);

    /**
     * @brief 发送错误信息
     * @param taskId 任务ID
     * @param errorCode 错误码
     * @param errorMessage 错误消息
     */
    void error(const QString &taskId, int errorCode, const QString &errorMessage);

    /**
     * @brief 发送警告信息
     * @param message 警告消息
     */
    void warning(const QString &message);

    /**
     * @brief 初始化Worker
     * @return 是否初始化成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 处理任务请求
     * @param task 任务消息
     * @return 处理结果
     */
    virtual bool processTask(const TaskMessage &task) = 0;

    /**
     * @brief 取消任务
     * @param taskId 任务ID
     */
    virtual void cancelTask(const QString &taskId) = 0;

    /**
     * @brief 关闭Worker
     */
    virtual void terminate() = 0;

    /**
     * @brief 处理收到的消息
     * @param data 消息数据
     * @return 是否处理成功
     */
    bool handleMessage(const QByteArray &data);

    /**
     * @brief 设置消息处理器
     * @param handler 消息处理函数
     */
    void setMessageHandler(std::function<void(const QByteArray &)> handler);

private:
    std::function<void(const QByteArray &)> m_messageHandler;
    QString m_workerId;
};

} // namespace Framework 