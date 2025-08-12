#include "CopyTask.h"
#include "CopyWorker.h"
#include "CopyAlgorithm.h"
#include "hooks/TaskHooks.h"
#include <QThread>
#include <QFileInfo>

CopyTask::CopyTask(const QString &source, const QString &destination,
                   CopyAlgorithm *algorithm, QObject *parent)
    : QObject(parent), m_source(source), m_destination(destination), m_algorithm(algorithm), m_hooks(nullptr), m_state(TaskState::Created), m_worker(nullptr)
{
    // Create worker and move to thread
    m_worker = new CopyWorker();
    m_worker->moveToThread(&m_workerThread);

    // Connect worker signals
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &CopyTask::startWorker, m_worker, &CopyWorker::startCopy);
    connect(this, &CopyTask::pauseWorker, this, [this]() {
        m_worker->pauseCopy();
    });
    connect(this, &CopyTask::resumeWorker, this, [this]() {
        m_worker->resumeCopy();
    });
    connect(this, &CopyTask::stopWorker, this, [this]() {
        m_worker->stopCopy();
    });

    // Connect worker result signals
    connect(m_worker, &CopyWorker::progressUpdated, this, &CopyTask::onWorkerProgressUpdated);
    connect(m_worker, &CopyWorker::fileStarted, this, &CopyTask::onWorkerFileStarted);
    connect(m_worker, &CopyWorker::fileCompleted, this, &CopyTask::onWorkerFileCompleted);
    connect(m_worker, &CopyWorker::copyError, this, &CopyTask::onWorkerError);
    connect(m_worker, &CopyWorker::copyCompleted, this, &CopyTask::onWorkerCompleted);
    connect(m_worker, &CopyWorker::copyPaused, this, &CopyTask::onWorkerPaused);
    connect(m_worker, &CopyWorker::copyResumed, this, &CopyTask::onWorkerResumed);
    connect(m_worker, &CopyWorker::copyStopped, this, &CopyTask::onWorkerStopped);

    m_workerThread.start();
}

CopyTask::~CopyTask()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

void CopyTask::start()
{
    if (!isValidStateTransition(m_state, TaskState::Running)) {
        return;
    }

    setState(TaskState::Running);

    // Call hook
    if (m_hooks) {
        m_hooks->onTaskStart(this);
    }

    // Start worker
    emit startWorker(m_source, m_destination, m_algorithm);
}

void CopyTask::pause()
{
    if (!isValidStateTransition(m_state, TaskState::Paused)) {
        return;
    }

    setState(TaskState::Paused);
    emit pauseWorker();

    // Call hook
    if (m_hooks) {
        m_hooks->onTaskPause(this);
    }
}

void CopyTask::resume()
{
    if (!isValidStateTransition(m_state, TaskState::Running)) {
        return;
    }

    setState(TaskState::Running);
    emit resumeWorker();

    // Call hook
    if (m_hooks) {
        m_hooks->onTaskResume(this);
    }
}

void CopyTask::stop()
{
    if (!isValidStateTransition(m_state, TaskState::Stopped)) {
        return;
    }

    setState(TaskState::Stopped);
    emit stopWorker();
}

TaskState CopyTask::getState() const
{
    return m_state;
}

CopyProgress CopyTask::getProgress() const
{
    return m_progress;
}

void CopyTask::setHooks(TaskHooks *hooks)
{
    m_hooks = hooks;
}

// Worker signal handlers
void CopyTask::onWorkerProgressUpdated(qint64 current, qint64 total)
{
    m_progress.copiedBytes = current;
    m_progress.totalBytes = total;
    emit progressChanged(m_progress);
}

void CopyTask::onWorkerFileStarted(const QString &filename)
{
    m_progress.currentFile = filename;
    emit progressChanged(m_progress);
}

void CopyTask::onWorkerFileCompleted(const QString &filename)
{
    Q_UNUSED(filename)
    emit progressChanged(m_progress);
}

void CopyTask::onWorkerError(const QString &error)
{
    m_errorMessage = error;
    setState(TaskState::Error);

    // Call hook
    if (m_hooks) {
        m_hooks->onTaskError(this, error);
    }

    emit errorOccurred(error);
}

void CopyTask::onWorkerCompleted()
{
    setState(TaskState::Completed);

    // Call hook
    if (m_hooks) {
        m_hooks->onTaskComplete(this);
    }

    emit finished();
}

void CopyTask::onWorkerPaused()
{
    // Worker has confirmed pause
}

void CopyTask::onWorkerResumed()
{
    // Worker has confirmed resume
}

void CopyTask::onWorkerStopped()
{
    if (m_state != TaskState::Stopped) {
        setState(TaskState::Stopped);
    }
    emit finished();
}

// ProgressObserver implementation (not used in new architecture but kept for compatibility)
void CopyTask::onProgressUpdate(qint64 current, qint64 total)
{
    onWorkerProgressUpdated(current, total);
}

void CopyTask::onFileStart(const QString &filename)
{
    onWorkerFileStarted(filename);
}

void CopyTask::onFileComplete(const QString &filename)
{
    onWorkerFileCompleted(filename);
}

void CopyTask::onError(const QString &error)
{
    onWorkerError(error);
}

void CopyTask::onComplete()
{
    onWorkerCompleted();
}

bool CopyTask::shouldPause() const
{
    return m_state == TaskState::Paused;
}

bool CopyTask::shouldStop() const
{
    return m_state == TaskState::Stopped;
}

void CopyTask::waitWhilePaused()
{
    // CopyTask doesn't implement waiting directly
    // This is handled by CopyWorker
}

void CopyTask::setState(TaskState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

bool CopyTask::isValidStateTransition(TaskState from, TaskState to) const
{
    switch (from) {
    case TaskState::Created:
        return to == TaskState::Running;

    case TaskState::Running:
        return to == TaskState::Paused || to == TaskState::Completed || to == TaskState::Stopped || to == TaskState::Error;

    case TaskState::Paused:
        return to == TaskState::Running || to == TaskState::Stopped;

    case TaskState::Completed:
    case TaskState::Stopped:
    case TaskState::Error:
        return false;   // Terminal states
    }

    return false;
}

#include "CopyTask.moc"
