#pragma once

#include <QObject>
#include <QMap>
#include <QThread>
#include <QProcess>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QPluginLoader>

#include "worker_base.h"

namespace Framework {

// 前向声明
class ThreadWorker;
class ProcessWorker;

/**
 * @brief Worker管理器，负责管理Worker的线程模式和进程模式
 */
class WorkerManager : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Worker运行模式
     */
    enum class RunMode {
        Thread,  // 在线程中运行
        Process  // 在独立进程中运行
    };

    explicit WorkerManager(QObject *parent = nullptr);
    ~WorkerManager();

    /**
     * @brief 加载Worker插件
     * @param pluginPath 插件路径
     * @param mode 运行模式
     * @return 是否成功加载
     */
    bool loadWorker(const QString &pluginPath, RunMode mode = RunMode::Thread);

    /**
     * @brief 加载目录中的所有Worker插件
     * @param directory 插件目录
     * @param mode 运行模式
     * @return 成功加载的插件数量
     */
    int loadWorkers(const QDir &directory, RunMode mode = RunMode::Thread);

    /**
     * @brief 卸载所有Worker
     */
    void unloadAllWorkers();

    /**
     * @brief 向Worker发送任务
     * @param workerId Worker标识符
     * @param task 任务消息
     * @return 是否成功发送
     */
    bool sendTask(const QString &workerId, const TaskMessage &task);

    /**
     * @brief 获取支持指定任务类型的Worker
     * @param taskType 任务类型
     * @return Worker标识符列表
     */
    QStringList getWorkersForTaskType(const QString &taskType) const;

    /**
     * @brief 获取所有已加载的Worker标识符
     * @return Worker标识符列表
     */
    QStringList getAllWorkerIds() const;

    /**
     * @brief 获取指定Worker的能力
     * @param workerId Worker标识符
     * @return 能力列表
     */
    QStringList getWorkerCapabilities(const QString &workerId) const;

    /**
     * @brief 检查指定Worker是否存在
     * @param workerId Worker标识符
     * @return 是否存在
     */
    bool hasWorker(const QString &workerId) const;

    /**
     * @brief 设置Worker进程可执行文件路径
     * @param path 可执行文件路径
     */
    void setWorkerProcessPath(const QString &path);

signals:
    /**
     * @brief Worker加载成功信号
     * @param workerId Worker标识符
     * @param mode 运行模式
     */
    void workerLoaded(const QString &workerId, RunMode mode);

    /**
     * @brief Worker加载失败信号
     * @param pluginPath 插件路径
     * @param errorMessage 错误消息
     */
    void workerLoadFailed(const QString &pluginPath, const QString &errorMessage);

    /**
     * @brief Worker任务进度更新信号
     * @param workerId Worker标识符
     * @param taskId 任务ID
     * @param progress 进度
     */
    void taskProgress(const QString &workerId, const QString &taskId, int progress);

    /**
     * @brief Worker任务完成信号
     * @param workerId Worker标识符
     * @param taskId 任务ID
     * @param success 是否成功
     * @param result 结果
     */
    void taskCompleted(const QString &workerId, const QString &taskId, bool success, const QVariant &result);

    /**
     * @brief Worker任务失败信号
     * @param workerId Worker标识符
     * @param taskId 任务ID
     * @param errorMessage 错误消息
     */
    void taskFailed(const QString &workerId, const QString &taskId, const QString &errorMessage);

private slots:
    /**
     * @brief 处理新的本地socket连接
     */
    void handleNewConnection();

    /**
     * @brief 处理进程Worker消息
     */
    void handleProcessWorkerMessage();

private:
    struct ThreadWorkerInfo {
        QThread *thread;
        WorkerBase *worker;
        QPluginLoader *loader;
    };

    struct ProcessWorkerInfo {
        QProcess *process;
        QLocalSocket *socket;
        QString pluginPath;
    };

    // 线程模式Worker列表
    QMap<QString, ThreadWorkerInfo> m_threadWorkers;
    
    // 进程模式Worker列表
    QMap<QString, ProcessWorkerInfo> m_processWorkers;
    
    // 本地服务器
    QLocalServer *m_server;
    
    // Worker进程可执行文件路径
    QString m_workerProcessPath;

    /**
     * @brief 加载线程模式Worker
     * @param pluginPath 插件路径
     * @return 是否成功加载
     */
    bool loadThreadWorker(const QString &pluginPath);

    /**
     * @brief 启动进程模式Worker
     * @param pluginPath 插件路径
     * @return 是否成功启动
     */
    bool startProcessWorker(const QString &pluginPath);

    /**
     * @brief 处理Worker消息
     * @param workerId Worker标识符
     * @param data 消息数据
     */
    void handleWorkerMessage(const QString &workerId, const QByteArray &data);

    /**
     * @brief 获取唯一的服务器名
     * @return 服务器名
     */
    QString getUniqueServerName() const;
};

} // namespace Framework 