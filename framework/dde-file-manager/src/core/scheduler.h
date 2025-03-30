#pragma once

#include <QObject>
#include <QQueue>
#include <QHash>
#include <QUrl>
#include <memory>
#include <functional>
#include <optional>

#include "job.h"
#include "worker.h"

namespace DFM {

// 调度器类 - 单例
class Scheduler : public QObject {
    Q_OBJECT
public:
    // 获取单例实例
    static Scheduler *instance();
    ~Scheduler();

    // 任务调度方法
    void scheduleJob(std::shared_ptr<Job> job);
    void cancelJob(std::shared_ptr<Job> job);
    
    // Worker池管理
    std::shared_ptr<Worker> getWorker(const QString &protocol, const QUrl &url);
    void releaseWorker(std::shared_ptr<Worker> worker);
    
    // 设置
    void setMaxWorkers(int count);
    int maxWorkers() const;

signals:
    void jobStarted(std::shared_ptr<Job> job);
    void jobFinished(std::shared_ptr<Job> job, bool success);

private:
    explicit Scheduler(QObject *parent = nullptr);
    void processQueue();
    
    class Private;
    std::unique_ptr<Private> d;
};

// Worker管理器 - 由调度器使用
class WorkerManager : public QObject {
    Q_OBJECT
public:
    explicit WorkerManager(QObject *parent = nullptr);
    ~WorkerManager();

    // Worker池管理
    std::shared_ptr<Worker> getIdleWorker(const QString &protocol, const QUrl &url);
    void releaseWorker(std::shared_ptr<Worker> worker);
    
    // Worker生命周期
    void terminateWorker(std::shared_ptr<Worker> worker);
    void cleanIdleWorkers(int maxIdleTime = 60000);

private slots:
    void onWorkerDied();

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace DFM 