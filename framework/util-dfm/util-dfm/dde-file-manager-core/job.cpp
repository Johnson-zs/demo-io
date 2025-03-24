#include "job.h"
#include <QDebug>

namespace DFM {

// 定义信号，解决链接错误
void Job::finished() {}
void Job::canceled() {}
void Job::error(int, const QString &) {}
void Job::percent(int) {}
void Job::processedSize(qint64, qint64) {}
void Job::speed(qint64) {}

class JobPrivate {
public:
    JobPrivate() 
        : error(0)
        , running(false)
        , flags(0)
    {}

    int error;                  // 错误代码
    QString errorText;          // 错误文本
    bool running;               // 是否正在运行
    JobFlags flags;             // 任务标志
};

Job::Job(QObject *parent)
    : QObject(parent)
    , d(new JobPrivate())
{
    qDebug() << "创建Job实例";
}

Job::~Job()
{
    qDebug() << "销毁Job实例";
    delete d;
}

int Job::error() const
{
    return d->error;
}

QString Job::errorText() const
{
    return d->errorText;
}

bool Job::isRunning() const
{
    return d->running;
}

JobFlags Job::flags() const
{
    return d->flags;
}

void Job::setFlags(JobFlags flags)
{
    d->flags = flags;
}

void Job::setError(int error)
{
    d->error = error;
}

void Job::setErrorText(const QString &errorText)
{
    d->errorText = errorText;
}

void Job::emitResult()
{
    d->running = false;
    Q_EMIT finished();
}

void Job::emitPercent(unsigned long percent)
{
    Q_EMIT this->percent(static_cast<int>(percent));
}

void Job::emitSpeed(unsigned long bytesPerSecond)
{
    Q_EMIT speed(static_cast<qint64>(bytesPerSecond));
}

bool Job::doSuspend()
{
    // 基类提供默认实现
    return true;
}

bool Job::doResume()
{
    // 基类提供默认实现
    return true;
}

bool Job::doKill()
{
    // 基类提供默认实现
    return true;
}

QString buildErrorString(int errorCode, const QString &errorText)
{
    return QString::number(errorCode) + ": " + errorText;
}

} // namespace DFM 