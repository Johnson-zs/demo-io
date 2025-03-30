#include "scheduler.h"
#include <QMutex>
#include <QMutexLocker>
#include <QHash>
#include <QQueue>
#include <QElapsedTimer>
#include <QDebug>
#include <QTimer>

namespace DFM {

class Scheduler::Private {
public:
    QQueue<std::shared_ptr<Job>> jobQueue;
    QHash<std::shared_ptr<Job>, std::shared_ptr<Worker>> jobWorkerMap;
    WorkerManager workerManager;
    QMutex mutex;
    int maxWorkers = 5;
    int runningJobs = 0;
};

// 单例实现
Scheduler *Scheduler::instance()
{
    static Scheduler instance;
    return &instance;
}

Scheduler::Scheduler(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->workerManager.setParent(this);
    
    // 定期清理空闲的worker
    QTimer *cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, [this]() {
        d->workerManager.cleanIdleWorkers();
    });
    cleanupTimer->start(60000); // 每分钟清理一次
}

Scheduler::~Scheduler()
{
    // 取消所有正在运行的任务
    QMutexLocker locker(&d->mutex);
    for (auto it = d->jobWorkerMap.begin(); it != d->jobWorkerMap.end(); ++it) {
        auto job = it.key();
        auto worker = it.value();
        
        job->cancel();
        d->workerManager.releaseWorker(worker);
    }
    
    d->jobWorkerMap.clear();
    d->jobQueue.clear();
}

void Scheduler::scheduleJob(std::shared_ptr<Job> job)
{
    if (!job) {
        return;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 将任务添加到队列
    d->jobQueue.enqueue(job);
    
    // 尝试处理队列
    processQueue();
}

void Scheduler::cancelJob(std::shared_ptr<Job> job)
{
    if (!job) {
        return;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 如果任务在队列中，直接移除
    if (d->jobQueue.contains(job)) {
        d->jobQueue.removeAll(job);
    }
    
    // 如果任务正在运行，取消并释放worker
    if (d->jobWorkerMap.contains(job)) {
        auto worker = d->jobWorkerMap[job];
        job->cancel();
        d->workerManager.releaseWorker(worker);
        d->jobWorkerMap.remove(job);
        d->runningJobs--;
        
        // 处理队列中的下一个任务
        processQueue();
    }
}

std::shared_ptr<Worker> Scheduler::getWorker(const QString &protocol, const QUrl &url)
{
    // 从WorkerManager获取worker
    return d->workerManager.getIdleWorker(protocol, url);
}

void Scheduler::releaseWorker(std::shared_ptr<Worker> worker)
{
    // 释放worker回到池中
    d->workerManager.releaseWorker(worker);
}

void Scheduler::setMaxWorkers(int count)
{
    if (count > 0) {
        d->maxWorkers = count;
    }
}

int Scheduler::maxWorkers() const
{
    return d->maxWorkers;
}

void Scheduler::processQueue()
{
    // 已经加锁，无需再次加锁
    
    // 处理队列中的任务，直到达到最大并发数或队列为空
    while (!d->jobQueue.isEmpty() && d->runningJobs < d->maxWorkers) {
        auto job = d->jobQueue.dequeue();
        
        // 对于SimpleJob，需要获取对应的Worker
        auto simpleJob = std::dynamic_pointer_cast<SimpleJob>(job);
        if (simpleJob) {
            QUrl url = simpleJob->url();
            QString protocol = url.scheme();
            
            // 获取适合的Worker
            auto worker = getWorker(protocol, url);
            if (!worker) {
                // 无法获取Worker，任务失败
                QString error = tr("Cannot find suitable worker for protocol: %1").arg(protocol);
                simpleJob->setErrorString(error);
                simpleJob->setState(JobState::Error);
                emit jobFinished(job, false);
                continue;
            }
            
            // 关联Worker和Job
            simpleJob->setWorker(worker);
            d->jobWorkerMap[job] = worker;
            d->runningJobs++;
            
            // 连接Job完成信号
            connect(job.get(), &Job::finished, this, [this, job](bool success) {
                QMutexLocker locker(&d->mutex);
                
                // 处理任务完成
                if (d->jobWorkerMap.contains(job)) {
                    auto worker = d->jobWorkerMap[job];
                    d->workerManager.releaseWorker(worker);
                    d->jobWorkerMap.remove(job);
                    d->runningJobs--;
                    
                    emit jobFinished(job, success);
                    
                    // 处理下一个任务
                    processQueue();
                }
            }, Qt::QueuedConnection);
            
            // 启动任务
            job->start();
            emit jobStarted(job);
        } else {
            // 处理非SimpleJob类型的任务
            d->runningJobs++;
            
            // 连接Job完成信号
            connect(job.get(), &Job::finished, this, [this, job](bool success) {
                QMutexLocker locker(&d->mutex);
                d->runningJobs--;
                emit jobFinished(job, success);
                
                // 处理下一个任务
                processQueue();
            }, Qt::QueuedConnection);
            
            // 启动任务
            job->start();
            emit jobStarted(job);
        }
    }
}

// WorkerManager Implementation
class WorkerManager::Private {
public:
    QHash<QString, QList<std::shared_ptr<Worker>>> idleWorkers; // 按协议分组的空闲worker
    QHash<std::shared_ptr<Worker>, QElapsedTimer> idleTime;     // worker空闲的时间
    QHash<std::shared_ptr<Worker>, bool> busyWorkers;          // 正在使用的worker
    QMutex mutex;
};

WorkerManager::WorkerManager(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

WorkerManager::~WorkerManager()
{
    // 终止所有worker
    QMutexLocker locker(&d->mutex);
    
    // 终止空闲worker
    for (auto it = d->idleWorkers.begin(); it != d->idleWorkers.end(); ++it) {
        for (auto worker : it.value()) {
            worker->terminate();
        }
    }
    
    // 终止忙碌worker
    for (auto it = d->busyWorkers.begin(); it != d->busyWorkers.end(); ++it) {
        it.key()->terminate();
    }
    
    d->idleWorkers.clear();
    d->busyWorkers.clear();
    d->idleTime.clear();
}

std::shared_ptr<Worker> WorkerManager::getIdleWorker(const QString &protocol, const QUrl &url)
{
    QMutexLocker locker(&d->mutex);
    
    // 检查是否有空闲的worker
    if (d->idleWorkers.contains(protocol) && !d->idleWorkers[protocol].isEmpty()) {
        // 获取空闲worker
        auto worker = d->idleWorkers[protocol].takeFirst();
        d->idleTime.remove(worker);
        d->busyWorkers[worker] = true;
        return worker;
    }
    
    // 需要创建新worker
    QString errorString;
    auto worker = Worker::createWorker(protocol, url, errorString);
    
    if (worker) {
        // 连接worker信号
        connect(worker.get(), &Worker::died, this, &WorkerManager::onWorkerDied);
        
        // 标记为忙碌
        d->busyWorkers[worker] = true;
    }
    
    return worker;
}

void WorkerManager::releaseWorker(std::shared_ptr<Worker> worker)
{
    if (!worker) {
        return;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 检查worker是否存活
    if (!worker->isAlive()) {
        terminateWorker(worker);
        return;
    }
    
    // 从忙碌列表移除
    d->busyWorkers.remove(worker);
    
    // 添加到空闲列表
    QString protocol = worker->protocol();
    d->idleWorkers[protocol].append(worker);
    
    // 记录空闲开始时间
    QElapsedTimer timer;
    timer.start();
    d->idleTime[worker] = timer;
}

void WorkerManager::terminateWorker(std::shared_ptr<Worker> worker)
{
    if (!worker) {
        return;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 从空闲和忙碌列表中移除
    QString protocol = worker->protocol();
    if (d->idleWorkers.contains(protocol)) {
        d->idleWorkers[protocol].removeAll(worker);
    }
    d->busyWorkers.remove(worker);
    d->idleTime.remove(worker);
    
    // 终止worker
    worker->terminate();
}

void WorkerManager::cleanIdleWorkers(int maxIdleTime)
{
    QMutexLocker locker(&d->mutex);
    
    QList<std::shared_ptr<Worker>> workersToRemove;
    
    // 检查每个空闲worker的空闲时间
    for (auto it = d->idleTime.begin(); it != d->idleTime.end(); ++it) {
        auto worker = it.key();
        auto &timer = it.value();
        
        if (timer.elapsed() > maxIdleTime) {
            workersToRemove.append(worker);
        }
    }
    
    // 终止并移除超时worker
    for (auto worker : workersToRemove) {
        QString protocol = worker->protocol();
        if (d->idleWorkers.contains(protocol)) {
            d->idleWorkers[protocol].removeAll(worker);
        }
        d->idleTime.remove(worker);
        
        // 终止worker
        worker->terminate();
    }
}

void WorkerManager::onWorkerDied()
{
    auto worker = qobject_cast<Worker*>(sender());
    if (!worker) {
        return;
    }
    
    // 将指针转换为shared_ptr，以便在哈希表中查找
    // 注意：这种方法不是完全安全的，可能需要更好的方式来跟踪worker
    std::shared_ptr<Worker> workerPtr;
    
    {
        QMutexLocker locker(&d->mutex);
        
        // 查找对应的shared_ptr
        for (auto it = d->busyWorkers.begin(); it != d->busyWorkers.end(); ++it) {
            if (it.key().get() == worker) {
                workerPtr = it.key();
                break;
            }
        }
        
        if (!workerPtr) {
            QString protocol = worker->protocol();
            if (d->idleWorkers.contains(protocol)) {
                for (auto &w : d->idleWorkers[protocol]) {
                    if (w.get() == worker) {
                        workerPtr = w;
                        break;
                    }
                }
            }
        }
        
        if (workerPtr) {
            terminateWorker(workerPtr);
        }
    }
}

} // namespace DFM 