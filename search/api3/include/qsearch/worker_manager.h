#pragma once

#include <QObject>
#include <QProcess>
#include <QMap>
#include "global.h"

namespace QSearch {

enum class WorkerType {
    Search,
    Index
};

class QSEARCH_EXPORT WorkerManager : public QObject {
    Q_OBJECT
public:
    static WorkerManager& instance();
    
    // 启动特定类型的worker进程
    bool startWorker(WorkerType type);
    
    // 停止worker进程
    bool stopWorker(WorkerType type);
    
    // 检查worker状态
    bool isWorkerRunning(WorkerType type) const;
    
    // 重启worker进程
    bool restartWorker(WorkerType type);
    
    // 设置worker进程资源限制
    void setWorkerResourceLimits(WorkerType type, int cpuLimit, int memoryLimit);
    
signals:
    void workerStarted(WorkerType type);
    void workerStopped(WorkerType type);
    void workerCrashed(WorkerType type);
    
private:
    // 私有构造函数(单例模式)
    WorkerManager(QObject* parent = nullptr);
    ~WorkerManager();
    
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 