#include "job.h"
#include "worker.h"
#include "connection.h"
#include <QPromise>
#include <QMutex>
#include <QMutexLocker>

namespace DFM {

class Job::Private {
public:
    Private() = default;
    
    JobState state = JobState::None;
    int progress = 0;
    QString errorString;
};

Job::Job(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

JobState Job::state() const
{
    return d->state;
}

int Job::progress() const
{
    return d->progress;
}

QString Job::errorString() const
{
    return d->errorString;
}

bool Job::suspend()
{
    if (d->state == JobState::Running) {
        setState(JobState::Paused);
        return true;
    }
    return false;
}

bool Job::resume()
{
    if (d->state == JobState::Paused) {
        setState(JobState::Running);
        return true;
    }
    return false;
}

bool Job::cancel()
{
    if (d->state == JobState::Running || d->state == JobState::Paused) {
        setState(JobState::Canceled);
        return true;
    }
    return false;
}

void Job::setState(JobState state)
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state);
        
        if (state == JobState::Finished || state == JobState::Error || state == JobState::Canceled) {
            emit finished(state == JobState::Finished);
        }
    }
}

void Job::setProgress(int progress)
{
    if (d->progress != progress) {
        d->progress = progress;
        emit progressChanged(progress);
    }
}

void Job::setErrorString(const QString &errorString)
{
    if (d->errorString != errorString) {
        d->errorString = errorString;
        emit errorStringChanged(errorString);
    }
}

// SimpleJob Implementation
class SimpleJob::Private {
public:
    Private(const QUrl &url)
        : url(url)
    {
    }
    
    QUrl url;
    std::shared_ptr<Worker> worker;
    QMutex mutex;
};

SimpleJob::SimpleJob(const QUrl &url, QObject *parent)
    : Job(parent)
    , d(std::make_unique<Private>(url))
{
}

SimpleJob::~SimpleJob()
{
    // 在析构时取消任务，确保资源被释放
    cancel();
}

QUrl SimpleJob::url() const
{
    return d->url;
}

void SimpleJob::setWorker(std::shared_ptr<Worker> worker)
{
    QMutexLocker locker(&d->mutex);
    d->worker = worker;
    
    // 连接worker的信号
    if (worker) {
        connect(worker.get(), &Worker::commandReceived, this, [this](int cmd, const QByteArray &data) {
            // 处理来自worker的命令
            if (cmd == Commands::MSG_PROGRESS) {
                QDataStream stream(data);
                int progress;
                stream >> progress;
                setProgress(progress);
            } else if (cmd == Commands::MSG_ERROR) {
                QDataStream stream(data);
                QString errorString;
                stream >> errorString;
                setErrorString(errorString);
                setState(JobState::Error);
            } else if (cmd == Commands::MSG_RESULT) {
                // 处理最终结果
                emit dataReceived(QVariant(data));
                setState(JobState::Finished);
            } else if (cmd == Commands::MSG_DATA) {
                // 处理中间数据
                emit dataReceived(QVariant(data));
            }
        });
        
        connect(worker.get(), &Worker::error, this, [this](const QString &errorString) {
            setErrorString(errorString);
            setState(JobState::Error);
        });
        
        connect(worker.get(), &Worker::died, this, [this]() {
            setErrorString(tr("Worker process died unexpectedly"));
            setState(JobState::Error);
        });
    }
}

void SimpleJob::sendCommand(int cmd, const QByteArray &data)
{
    QMutexLocker locker(&d->mutex);
    if (d->worker) {
        d->worker->sendCommand(cmd, data);
    }
}

} // namespace DFM 