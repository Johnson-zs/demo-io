#ifndef COPYTASKMANAGER_H
#define COPYTASKMANAGER_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QString>
#include <QStringList>

// Forward declarations
class CopyTask;
class CopyAlgorithm;

/**
 * @brief Manages copy tasks and algorithms
 *
 * This class is responsible for:
 * - Managing available copy algorithms
 * - Creating and managing copy tasks
 * - Coordinating task execution
 * - Providing a centralized interface for the UI
 */
class CopyTaskManager : public QObject
{
    Q_OBJECT

public:
    explicit CopyTaskManager(QObject *parent = nullptr);
    virtual ~CopyTaskManager();

    // Algorithm management
    void registerAlgorithm(const QString &name, CopyAlgorithm *algorithm);
    void unregisterAlgorithm(const QString &name);
    QStringList getAvailableAlgorithms() const;
    CopyAlgorithm *getAlgorithm(const QString &name) const;

    // Task management
    CopyTask *createTask(const QString &source, const QString &destination,
                         const QString &algorithmName);
    void startTask(CopyTask *task);
    void pauseTask(CopyTask *task);
    void resumeTask(CopyTask *task);
    void stopTask(CopyTask *task);
    void removeTask(CopyTask *task);

    // Getters
    QList<CopyTask *> getAllTasks() const;
    CopyTask *getActiveTask() const;
    int getTaskCount() const;

    // Utility methods
    void clearCompletedTasks();
    void stopAllTasks();

signals:
    void taskCreated(CopyTask *task);
    void taskStarted(CopyTask *task);
    void taskCompleted(CopyTask *task);
    void taskError(CopyTask *task, const QString &error);
    void algorithmRegistered(const QString &name);
    void algorithmUnregistered(const QString &name);

private slots:
    void onTaskStateChanged();
    void onTaskFinished();

private:
    void connectTaskSignals(CopyTask *task);
    void disconnectTaskSignals(CopyTask *task);

    QHash<QString, CopyAlgorithm *> m_algorithms;
    QList<CopyTask *> m_tasks;
    CopyTask *m_activeTask;
};

#endif   // COPYTASKMANAGER_H
