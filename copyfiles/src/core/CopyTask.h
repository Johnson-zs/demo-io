#ifndef COPYTASK_H
#define COPYTASK_H

#include <QObject>
#include <QString>
#include <QThread>
#include "ProgressObserver.h"

// Forward declarations
class CopyAlgorithm;
class TaskHooks;
class CopyWorker;

/**
 * @brief Enumeration of task states
 */
enum class TaskState {
    Created,   // Task created but not started
    Running,   // Task is currently running
    Paused,   // Task is paused
    Completed,   // Task completed successfully
    Stopped,   // Task was stopped by user
    Error   // Task encountered an error
};

/**
 * @brief Structure to hold copy progress information
 */
struct CopyProgress
{
    qint64 totalBytes = 0;
    qint64 copiedBytes = 0;
    QString currentFile;
    double percentage() const
    {
        return totalBytes > 0 ? (static_cast<double>(copiedBytes) / totalBytes) * 100.0 : 0.0;
    }
};

/**
 * @brief Represents a file copy task
 *
 * This class manages the execution of a copy operation using a specific
 * algorithm. It provides state management, progress tracking, and supports
 * pause/resume/stop operations.
 */
class CopyTask : public QObject, public ProgressObserver
{
    Q_OBJECT

public:
    explicit CopyTask(const QString &source, const QString &destination,
                      CopyAlgorithm *algorithm, QObject *parent = nullptr);
    virtual ~CopyTask();

    // Task control methods
    void start();
    void pause();
    void resume();
    void stop();

    // Getters
    TaskState getState() const;
    CopyProgress getProgress() const;
    QString getSource() const { return m_source; }
    QString getDestination() const { return m_destination; }
    QString getErrorMessage() const { return m_errorMessage; }

    // Hook management
    void setHooks(TaskHooks *hooks);
    TaskHooks *getHooks() const { return m_hooks; }

    // ProgressObserver implementation
    void onProgressUpdate(qint64 current, qint64 total) override;
    void onFileStart(const QString &filename) override;
    void onFileComplete(const QString &filename) override;
    void onError(const QString &error) override;
    void onComplete() override;
    bool shouldPause() const override;
    bool shouldStop() const override;
    void waitWhilePaused() override;

signals:
    void stateChanged(TaskState newState);
    void progressChanged(const CopyProgress &progress);
    void errorOccurred(const QString &error);
    void finished();

    // Worker control signals
    void startWorker(const QString &source, const QString &destination, CopyAlgorithm *algorithm);
    void pauseWorker();
    void resumeWorker();
    void stopWorker();

private slots:
    void onWorkerProgressUpdated(qint64 current, qint64 total);
    void onWorkerFileStarted(const QString &filename);
    void onWorkerFileCompleted(const QString &filename);
    void onWorkerError(const QString &error);
    void onWorkerCompleted();
    void onWorkerPaused();
    void onWorkerResumed();
    void onWorkerStopped();

private:
    void setState(TaskState newState);
    bool isValidStateTransition(TaskState from, TaskState to) const;

    QString m_source;
    QString m_destination;
    CopyAlgorithm *m_algorithm;
    TaskHooks *m_hooks;

    TaskState m_state;
    CopyProgress m_progress;
    QString m_errorMessage;

    QThread m_workerThread;
    CopyWorker *m_worker;
};

#endif   // COPYTASK_H
