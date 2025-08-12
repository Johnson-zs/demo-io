#include "CopyTaskManager.h"
#include "CopyTask.h"
#include "CopyAlgorithm.h"

CopyTaskManager::CopyTaskManager(QObject* parent)
    : QObject(parent)
    , m_activeTask(nullptr)
{
}

CopyTaskManager::~CopyTaskManager()
{
    stopAllTasks();
    
    // Clean up tasks
    qDeleteAll(m_tasks);
    m_tasks.clear();
    
    // Clean up algorithms
    qDeleteAll(m_algorithms);
    m_algorithms.clear();
}

void CopyTaskManager::registerAlgorithm(const QString& name, CopyAlgorithm* algorithm)
{
    if (!algorithm || name.isEmpty()) {
        return;
    }
    
    // Remove existing algorithm with same name
    if (m_algorithms.contains(name)) {
        delete m_algorithms[name];
    }
    
    algorithm->setParent(this);
    m_algorithms[name] = algorithm;
    
    emit algorithmRegistered(name);
}

void CopyTaskManager::unregisterAlgorithm(const QString& name)
{
    if (m_algorithms.contains(name)) {
        delete m_algorithms[name];
        m_algorithms.remove(name);
        emit algorithmUnregistered(name);
    }
}

QStringList CopyTaskManager::getAvailableAlgorithms() const
{
    return m_algorithms.keys();
}

CopyAlgorithm* CopyTaskManager::getAlgorithm(const QString& name) const
{
    return m_algorithms.value(name, nullptr);
}

CopyTask* CopyTaskManager::createTask(const QString& source, const QString& destination, 
                                     const QString& algorithmName)
{
    CopyAlgorithm* algorithm = getAlgorithm(algorithmName);
    if (!algorithm) {
        return nullptr;
    }
    
    // Create a new instance of the algorithm for this task
    // This ensures each task has its own algorithm instance
    CopyAlgorithm* algorithmInstance = nullptr;
    
    // For now, we'll use the same instance
    // In a more sophisticated implementation, we might clone the algorithm
    algorithmInstance = algorithm;
    
    CopyTask* task = new CopyTask(source, destination, algorithmInstance, this);
    
    connectTaskSignals(task);
    m_tasks.append(task);
    
    emit taskCreated(task);
    
    return task;
}

void CopyTaskManager::startTask(CopyTask* task)
{
    if (!task || !m_tasks.contains(task)) {
        return;
    }
    
    // Only allow one active task at a time for now
    if (m_activeTask && m_activeTask != task) {
        // Could pause the current task or queue this one
        // For simplicity, we'll just return
        return;
    }
    
    m_activeTask = task;
    task->start();
    
    emit taskStarted(task);
}

void CopyTaskManager::pauseTask(CopyTask* task)
{
    if (!task || !m_tasks.contains(task)) {
        return;
    }
    
    task->pause();
}

void CopyTaskManager::resumeTask(CopyTask* task)
{
    if (!task || !m_tasks.contains(task)) {
        return;
    }
    
    task->resume();
}

void CopyTaskManager::stopTask(CopyTask* task)
{
    if (!task || !m_tasks.contains(task)) {
        return;
    }
    
    task->stop();
    
    if (m_activeTask == task) {
        m_activeTask = nullptr;
    }
}

void CopyTaskManager::removeTask(CopyTask* task)
{
    if (!task || !m_tasks.contains(task)) {
        return;
    }
    
    // Stop task if it's running
    if (task->getState() == TaskState::Running || 
        task->getState() == TaskState::Paused) {
        stopTask(task);
    }
    
    disconnectTaskSignals(task);
    m_tasks.removeOne(task);
    
    if (m_activeTask == task) {
        m_activeTask = nullptr;
    }
    
    task->deleteLater();
}

QList<CopyTask*> CopyTaskManager::getAllTasks() const
{
    return m_tasks;
}

CopyTask* CopyTaskManager::getActiveTask() const
{
    return m_activeTask;
}

int CopyTaskManager::getTaskCount() const
{
    return m_tasks.size();
}

void CopyTaskManager::clearCompletedTasks()
{
    QList<CopyTask*> completedTasks;
    
    for (CopyTask* task : m_tasks) {
        TaskState state = task->getState();
        if (state == TaskState::Completed || 
            state == TaskState::Stopped || 
            state == TaskState::Error) {
            completedTasks.append(task);
        }
    }
    
    for (CopyTask* task : completedTasks) {
        removeTask(task);
    }
}

void CopyTaskManager::stopAllTasks()
{
    for (CopyTask* task : m_tasks) {
        if (task->getState() == TaskState::Running || 
            task->getState() == TaskState::Paused) {
            task->stop();
        }
    }
    
    m_activeTask = nullptr;
}

void CopyTaskManager::onTaskStateChanged()
{
    CopyTask* task = qobject_cast<CopyTask*>(sender());
    if (!task) {
        return;
    }
    
    TaskState state = task->getState();
    
    switch (state) {
    case TaskState::Completed:
        if (m_activeTask == task) {
            m_activeTask = nullptr;
        }
        emit taskCompleted(task);
        break;
        
    case TaskState::Error:
        if (m_activeTask == task) {
            m_activeTask = nullptr;
        }
        emit taskError(task, task->getErrorMessage());
        break;
        
    case TaskState::Stopped:
        if (m_activeTask == task) {
            m_activeTask = nullptr;
        }
        break;
        
    default:
        break;
    }
}

void CopyTaskManager::onTaskFinished()
{
    CopyTask* task = qobject_cast<CopyTask*>(sender());
    if (!task) {
        return;
    }
    
    if (m_activeTask == task) {
        m_activeTask = nullptr;
    }
}

void CopyTaskManager::connectTaskSignals(CopyTask* task)
{
    if (!task) {
        return;
    }
    
    connect(task, &CopyTask::stateChanged, 
            this, &CopyTaskManager::onTaskStateChanged);
    connect(task, &CopyTask::finished, 
            this, &CopyTaskManager::onTaskFinished);
}

void CopyTaskManager::disconnectTaskSignals(CopyTask* task)
{
    if (!task) {
        return;
    }
    
    disconnect(task, nullptr, this, nullptr);
}

