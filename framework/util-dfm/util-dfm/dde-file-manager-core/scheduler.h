#ifndef DFM_SCHEDULER_H
#define DFM_SCHEDULER_H

#include <QObject>
#include <QHash>
#include <QUrl>

namespace DFM {

class SimpleJob;
class Worker;
class SchedulerPrivate;

/**
 * @class Scheduler
 * @brief 任务调度器类
 * 
 * Scheduler类负责管理Worker实例和分配任务。
 * 它维护Worker池并根据需要创建新的Worker。
 */
class Scheduler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取调度器实例
     * @return 调度器实例
     */
    static Scheduler *self();

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Scheduler(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~Scheduler();

    /**
     * @brief 获取空闲Worker实例
     * @param protocol 协议
     * @param url URL
     * @param error 错误码
     * @param error_text 错误文本
     * @return Worker实例
     */
    Worker *getWorker(const QString &protocol, const QUrl &url, int &error, QString &error_text);
    
    /**
     * @brief 注册任务
     * @param job 任务
     * @param url URL
     */
    void registerJob(SimpleJob *job, const QUrl &url);
    
    /**
     * @brief 取消任务
     * @param job 任务
     */
    void cancelJob(SimpleJob *job);
    
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

Q_SIGNALS:
    /**
     * @brief 重新解析Worker配置的信号
     * @param proto 协议
     */
    void reparseSlaveConfiguration(const QString &proto);

private Q_SLOTS:
    /**
     * @brief 处理Worker死亡事件
     * @param worker Worker实例
     */
    void slotWorkerDied(Worker *worker);

private:
    std::unique_ptr<SchedulerPrivate> d;  ///< 私有实现
};

} // namespace DFM

#endif // DFM_SCHEDULER_H 