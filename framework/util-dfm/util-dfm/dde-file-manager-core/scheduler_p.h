#ifndef DFM_SCHEDULER_P_H
#define DFM_SCHEDULER_P_H

#include "scheduler.h"
#include <QHash>
#include <QMap>
#include <QUrl>
#include <QSet>
#include <unordered_map>

namespace DFM {

class SimpleJob;
class Worker;
class Scheduler;

/**
 * @class HostQueue
 * @brief 主机队列类
 * 
 * HostQueue类管理与特定主机关联的Worker。
 */
class HostQueue {
public:
    QList<Worker *> workers;   ///< Worker列表
    QSet<SimpleJob *> jobs;    ///< 任务集合
    int runningJobs = 0;        ///< 运行中的任务数
};

/**
 * @class ProtoQueue
 * @brief 协议队列类
 * 
 * ProtoQueue类管理与特定协议关联的主机队列。
 */
class ProtoQueue {
public:
    /**
     * @brief 构造函数
     * @param maxWorkers 最大Worker数
     * @param maxWorkersPerHost 每个主机的最大Worker数
     */
    ProtoQueue(int maxWorkers, int maxWorkersPerHost);
    
    /**
     * @brief 析构函数
     */
    ~ProtoQueue();

    int maxWorkers;              ///< 最大Worker数
    int maxWorkersPerHost;       ///< 每个主机的最大Worker数
    std::unordered_map<QString, HostQueue> queues;  ///< 主机队列表
    QList<Worker *> idleWorkers;  ///< 空闲Worker列表
    QList<Worker *> allWorkers;   ///< 所有Worker列表
};

/**
 * @class SchedulerPrivate
 * @brief Scheduler的私有实现
 */
class SchedulerPrivate {
public:
    /**
     * @brief 构造函数
     */
    SchedulerPrivate();
    
    /**
     * @brief 析构函数
     */
    ~SchedulerPrivate();
    
    SchedulerPrivate(const SchedulerPrivate &) = delete;
    SchedulerPrivate &operator=(const SchedulerPrivate &) = delete;

    /**
     * @brief 执行任务
     * @param job 任务
     */
    void doJob(SimpleJob *job);
    
    /**
     * @brief 取消任务
     * @param job 任务
     */
    void cancelJob(SimpleJob *job);
    
    /**
     * @brief 处理任务完成事件
     * @param job 任务
     * @param worker Worker实例
     */
    void jobFinished(SimpleJob *job, Worker *worker);
    
    /**
     * @brief 将Worker置于保持状态
     * @param job 任务
     * @param url URL
     */
    void putWorkerOnHold(SimpleJob *job, const QUrl &url);
    
    /**
     * @brief 移除保持状态的Worker
     */
    void removeWorkerOnHold();
    
    /**
     * @brief 检查Worker是否处于特定URL的保持状态
     * @param url URL
     * @return 如果处于保持状态返回true
     */
    bool isWorkerOnHoldFor(const QUrl &url);
    
    /**
     * @brief 更新内部元数据
     * @param job 任务
     */
    void updateInternalMetaData(SimpleJob *job);
    
    /**
     * @brief 获取元数据
     * @param protocol 协议
     * @param url URL
     * @return 元数据
     */
    QMap<QString, QString> metaDataFor(const QString &protocol, const QUrl &url);
    
    /**
     * @brief 设置Worker
     * @param worker Worker实例
     * @param url URL
     * @param protocol 协议
     * @param newWorker 是否是新Worker
     * @param config 配置
     */
    void setupWorker(Worker *worker, const QUrl &url, const QString &protocol, bool newWorker, 
                    const QMap<QString, QString> *config = nullptr);
    
    /**
     * @brief 处理Worker死亡事件
     * @param worker Worker实例
     */
    void slotWorkerDied(Worker *worker);
    
    /**
     * @brief 处理重新解析Worker配置事件
     * @param proto 协议
     */
    void slotReparseSlaveConfiguration(const QString &proto);

    Scheduler *q;                      ///< 公共类指针
    QHash<QString, ProtoQueue *> m_protocols;  ///< 协议队列
    QUrl m_urlOnHold;                  ///< 保持状态的URL
    bool m_ignoreConfigReparse = false;  ///< 是否忽略配置重新解析
};

} // namespace DFM

#endif // DFM_SCHEDULER_P_H 