#ifndef COPYWORKER_H
#define COPYWORKER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QWaitCondition>
#include "ProgressObserver.h"

// Forward declarations
class CopyAlgorithm;

/**
 * @brief Worker class for performing copy operations in a separate thread
 * 
 * This class handles the actual copy work and emits signals to update
 * the UI thread with progress information.
 */
class CopyWorker : public QObject, public ProgressObserver
{
    Q_OBJECT

public:
    explicit CopyWorker(QObject* parent = nullptr);
    virtual ~CopyWorker() = default;

    // ProgressObserver implementation
    void onProgressUpdate(qint64 current, qint64 total) override;
    void onFileStart(const QString& filename) override;
    void onFileComplete(const QString& filename) override;
    void onError(const QString& error) override;
    void onComplete() override;
    bool shouldPause() const override;
    bool shouldStop() const override;
    void waitWhilePaused() override;

public slots:
    void startCopy(const QString& source, const QString& destination, CopyAlgorithm* algorithm);
    void pauseCopy();
    void resumeCopy();
    void stopCopy();

signals:
    void progressUpdated(qint64 current, qint64 total, const QString& currentFile);
    void fileStarted(const QString& filename);
    void fileCompleted(const QString& filename);
    void copyError(const QString& error);
    void copyCompleted();
    void copyPaused();
    void copyResumed();
    void copyStopped();

private:
    // Control state
    enum class WorkerState {
        Running,
        Paused,
        Stopped
    };
    
    CopyAlgorithm* m_algorithm;
    WorkerState m_state;
    QString m_currentSource;
    QString m_currentDestination;
    
    // Thread synchronization
    mutable QMutex m_mutex;
    QWaitCondition m_pauseCondition;
    
    // Helper methods
    void checkPauseState();
    bool isStopRequested() const;
    void cleanupIncompleteFile(const QString& filePath);
};

#endif // COPYWORKER_H 