#include "scheduler.h"
#include "scheduler_p.h"
#include "worker.h"
#include "simplejob.h"
#include "errorcodes.h"
#include <QDebug>

namespace DFM {

// 单例实例
static Scheduler *s_self = nullptr;

// ProtoQueue实现
ProtoQueue::ProtoQueue(int maxWorkers, int maxWorkersPerHost)
    : maxWorkers(maxWorkers)
    , maxWorkersPerHost(maxWorkersPerHost)
{
}

ProtoQueue::~ProtoQueue()
{
    // 清理所有Worker
    qDeleteAll(allWorkers);
    allWorkers.clear();
    idleWorkers.clear();
    queues.clear();
}

// SchedulerPrivate实现
SchedulerPrivate::SchedulerPrivate()
    : q(nullptr)
    , m_ignoreConfigReparse(false)
{
}

SchedulerPrivate::~SchedulerPrivate()
{
    // 清理所有协议队列
    for (auto it = m_protocols.begin(); it != m_protocols.end(); ++it) {
        delete it.value();
    }
    m_protocols.clear();
}

void SchedulerPrivate::doJob(SimpleJob *job)
{
    if (!job) {
        return;
    }

    // 获取任务URL和协议
    QUrl url = job->url();
    QString protocol = url.scheme();
    QString host = url.host();

    // 尝试查找适合的Worker
    int error = 0;
    QString errorText;
    Worker *worker = q->getWorker(protocol, url, error, errorText);

    if (!worker) {
        // 如果没有找到可用的Worker，发送错误
        job->slotError(error, errorText);
        return;
    }

    // 检查URL是否与保持状态的Worker相匹配
    if (isWorkerOnHoldFor(url)) {
        removeWorkerOnHold();
    }

    // 关联任务与Worker
    worker->setJob(job);
    job->setWorker(worker);

    // 如果是新创建的Worker，检查是否需要特殊设置
    setupWorker(worker, url, protocol, false);

    // 添加到相应的队列中
    if (!host.isEmpty()) {
        ProtoQueue *pq = m_protocols.value(protocol);
        if (!pq) {
            // 如果该协议没有队列，创建一个新队列
            pq = new ProtoQueue(5, 2); // 默认最多5个Worker，每个主机最多2个
            m_protocols.insert(protocol, pq);
        }

        auto &hostQueue = pq->queues[host];
        hostQueue.workers.append(worker);
        hostQueue.jobs.insert(job);
        hostQueue.runningJobs++;
    }
}

void SchedulerPrivate::cancelJob(SimpleJob *job)
{
    // 如果任务已经有关联的Worker，取消它
    Worker *worker = job->worker();
    if (worker) {
        worker->setJob(nullptr);
        job->setWorker(nullptr);

        // 将Worker移回空闲队列
        QString protocol = worker->protocol();
        ProtoQueue *pq = m_protocols.value(protocol);
        if (pq) {
            pq->idleWorkers.append(worker);
            
            // 从主机队列中移除
            QString host = worker->host();
            if (!host.isEmpty() && pq->queues.count(host) > 0) {
                auto &hostQueue = pq->queues[host];
                hostQueue.jobs.remove(job);
                hostQueue.runningJobs--;
            }
        }
    }
}

void SchedulerPrivate::jobFinished(SimpleJob *job, Worker *worker)
{
    if (!job || !worker) {
        return;
    }
    
    // 清理任务与Worker的关联
    worker->setJob(nullptr);
    job->setWorker(nullptr);
    
    // 将Worker设置为空闲
    worker->setIdle();
    
    // 更新相应队列
    QString protocol = worker->protocol();
    QString host = worker->host();
    
    ProtoQueue *pq = m_protocols.value(protocol);
    if (pq) {
        pq->idleWorkers.append(worker);
        
        // 更新主机队列
        if (!host.isEmpty() && pq->queues.count(host) > 0) {
            auto &hostQueue = pq->queues[host];
            hostQueue.jobs.remove(job);
            hostQueue.runningJobs--;
        }
    }
}

void SchedulerPrivate::putWorkerOnHold(SimpleJob *job, const QUrl &url)
{
    Worker *worker = job->worker();
    if (!worker) {
        return;
    }
    
    // 将URL设置为保持状态
    m_urlOnHold = url;
    
    // 挂起Worker但不断开连接
    worker->suspend();
    
    // 清理任务与Worker的关联
    worker->setJob(nullptr);
    job->setWorker(nullptr);
}

void SchedulerPrivate::removeWorkerOnHold()
{
    if (m_urlOnHold.isEmpty()) {
        return;
    }
    
    // 清除保持状态的URL
    m_urlOnHold = QUrl();
    
    // 可能需要额外的清理操作
}

bool SchedulerPrivate::isWorkerOnHoldFor(const QUrl &url)
{
    return !m_urlOnHold.isEmpty() && m_urlOnHold == url;
}

void SchedulerPrivate::updateInternalMetaData(SimpleJob *job)
{
    // 更新任务的内部元数据
    if (!job) {
        return;
    }
    
    // 实际实现可能需要更复杂的逻辑
}

QMap<QString, QString> SchedulerPrivate::metaDataFor(const QString &protocol, const QUrl &url)
{
    QMap<QString, QString> metaData;
    
    // 添加基本元数据
    metaData["Protocol"] = protocol;
    metaData["URL"] = url.toString();
    
    // 可能需要添加更多与特定协议相关的元数据
    
    return metaData;
}

void SchedulerPrivate::setupWorker(Worker *worker, const QUrl &url, const QString &protocol, bool newWorker, const QMap<QString, QString> *config)
{
    if (!worker) {
        return;
    }
    
    // 设置Worker的基本属性
    if (newWorker) {
        worker->setProtocol(protocol);
    }
    
    // 设置主机信息
    if (!url.host().isEmpty()) {
        worker->setHost(url.host(), url.port(), url.userName(), url.password());
    } else {
        worker->resetHost();
    }
    
    // 如果提供了配置，设置配置
    if (config) {
        worker->setConfig(*config);
    }
}

void SchedulerPrivate::slotWorkerDied(Worker *worker)
{
    if (!worker) {
        return;
    }
    
    // 从所有队列中移除Worker
    QString protocol = worker->protocol();
    ProtoQueue *pq = m_protocols.value(protocol);
    if (pq) {
        pq->idleWorkers.removeAll(worker);
        pq->allWorkers.removeAll(worker);
        
        // 从主机队列中移除
        QString host = worker->host();
        if (!host.isEmpty() && pq->queues.count(host) > 0) {
            auto &hostQueue = pq->queues[host];
            hostQueue.workers.removeAll(worker);
        }
    }
    
    // 将任务设置为错误状态
    SimpleJob *job = worker->job();
    if (job) {
        job->slotError(ERR_WORKER_DIED, "Worker process died unexpectedly");
        job->setWorker(nullptr);
    }
}

void SchedulerPrivate::slotReparseSlaveConfiguration(const QString &proto)
{
    if (m_ignoreConfigReparse) {
        return;
    }
    
    // 对特定协议的所有Worker重新加载配置
    ProtoQueue *pq = m_protocols.value(proto);
    if (pq) {
        for (Worker *worker : pq->allWorkers) {
            // 重新加载Worker配置
            QMap<QString, QString> config = metaDataFor(proto, QUrl());
            worker->setConfig(config);
        }
    }
}

// Scheduler实现
Scheduler *Scheduler::self()
{
    if (!s_self) {
        s_self = new Scheduler();
    }
    return s_self;
}

Scheduler::Scheduler(QObject *parent)
    : QObject(parent)
    , d(new SchedulerPrivate())
{
    d->q = this;
}

Scheduler::~Scheduler()
{
    if (s_self == this) {
        s_self = nullptr;
    }
}

Worker *Scheduler::getWorker(const QString &protocol, const QUrl &url, int &error, QString &error_text)
{
    // 检查协议队列
    ProtoQueue *pq = d->m_protocols.value(protocol);
    if (!pq) {
        // 如果没有该协议的队列，创建一个新队列
        pq = new ProtoQueue(5, 2); // 默认最多5个Worker，每个主机最多2个
        d->m_protocols.insert(protocol, pq);
    }
    
    // 首先查找空闲的Worker
    for (Worker *worker : pq->idleWorkers) {
        if (worker->protocol() == protocol) {
            pq->idleWorkers.removeAll(worker);
            return worker;
        }
    }
    
    // 如果没有找到空闲的Worker，创建新的Worker
    Worker *worker = Worker::createWorker(protocol, url, error, error_text);
    if (worker) {
        // 连接信号
        connect(worker, &Worker::workerDied, this, [this, worker]() {
            d->slotWorkerDied(worker);
        });
        
        // 添加到队列
        pq->allWorkers.append(worker);
    }
    
    return worker;
}

void Scheduler::registerJob(SimpleJob *job, const QUrl &url)
{
    if (!job) {
        return;
    }
    
    // 设置任务URL
    job->setUrl(url);
    
    // 开始调度任务
    d->doJob(job);
}

void Scheduler::cancelJob(SimpleJob *job)
{
    if (!job) {
        return;
    }
    
    // 取消任务
    d->cancelJob(job);
}

void Scheduler::putWorkerOnHold(SimpleJob *job, const QUrl &url)
{
    if (!job) {
        return;
    }
    
    // 将Worker置于保持状态
    d->putWorkerOnHold(job, url);
}

void Scheduler::removeWorkerOnHold()
{
    // 移除保持状态的Worker
    d->removeWorkerOnHold();
}

void Scheduler::slotWorkerDied(Worker *worker)
{
    // 处理Worker死亡事件
    d->slotWorkerDied(worker);
}

} // namespace DFM 