#include "CopyWorker.h"
#include "CopyAlgorithm.h"
#include <QFileInfo>
#include <QThread>
#include <QFile>
#include <QMutexLocker>

CopyWorker::CopyWorker(QObject *parent)
    : QObject(parent), m_algorithm(nullptr), m_state(WorkerState::Running)
{
}

void CopyWorker::startCopy(const QString &source, const QString &destination, CopyAlgorithm *algorithm)
{
    QMutexLocker locker(&m_mutex);

    m_currentSource = source;
    m_currentDestination = destination;
    m_algorithm = algorithm;
    m_state = WorkerState::Running;

    locker.unlock();

    if (!m_algorithm) {
        emit copyError("No algorithm provided");
        return;
    }

    // Perform the copy operation
    QFileInfo sourceInfo(source);
    bool success = false;

    if (sourceInfo.isFile()) {
        success = m_algorithm->copyFile(source, destination, this);
    } else if (sourceInfo.isDir()) {
        success = m_algorithm->copyDirectory(source, destination, this);
    } else {
        emit copyError("Source is neither a file nor directory");
        return;
    }

    // Check final state
    locker.relock();
    if (m_state == WorkerState::Stopped) {
        emit copyStopped();
    } else if (success) {
        emit copyCompleted();
    } else {
        emit copyError("Copy operation failed");
    }
}

void CopyWorker::pauseCopy()
{
    QMutexLocker locker(&m_mutex);
    if (m_state == WorkerState::Running) {
        m_state = WorkerState::Paused;
        emit copyPaused();
    }
}

void CopyWorker::resumeCopy()
{
    QMutexLocker locker(&m_mutex);
    if (m_state == WorkerState::Paused) {
        m_state = WorkerState::Running;
        m_pauseCondition.wakeAll();
        emit copyResumed();
    }
}

void CopyWorker::stopCopy()
{
    QMutexLocker locker(&m_mutex);
    m_state = WorkerState::Stopped;
    m_pauseCondition.wakeAll();   // Wake up if paused
}

void CopyWorker::checkPauseState()
{
    QMutexLocker locker(&m_mutex);
    while (m_state == WorkerState::Paused) {
        m_pauseCondition.wait(&m_mutex);
    }
}

bool CopyWorker::isStopRequested() const
{
    QMutexLocker locker(&m_mutex);
    return m_state == WorkerState::Stopped;
}

void CopyWorker::cleanupIncompleteFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.exists()) {
        file.remove();
    }
}

// ProgressObserver interface implementations
void CopyWorker::onProgressUpdate(qint64 copiedBytes, qint64 totalBytes)
{
    emit progressUpdated(copiedBytes, totalBytes, m_currentSource);
}

void CopyWorker::onFileStart(const QString &filePath)
{
    // Check pause/stop state before starting a new file
    checkPauseState();

    if (isStopRequested()) {
        return;
    }

    emit fileStarted(filePath);
}

void CopyWorker::onFileComplete(const QString &filePath)
{
    emit fileCompleted(filePath);
}

void CopyWorker::onError(const QString &message)
{
    emit copyError(message);
}

void CopyWorker::onComplete()
{
    // This will be handled in startCopy() method
}

bool CopyWorker::shouldPause() const
{
    QMutexLocker locker(&m_mutex);
    return m_state == WorkerState::Paused;
}

bool CopyWorker::shouldStop() const
{
    QMutexLocker locker(&m_mutex);
    return m_state == WorkerState::Stopped;
}

void CopyWorker::waitWhilePaused()
{
    QMutexLocker locker(&m_mutex);
    while (m_state == WorkerState::Paused) {
        m_pauseCondition.wait(&m_mutex);
    }
}
