#include "simplejob.h"
#include "worker.h"
#include "scheduler.h"
#include "commands.h"
#include "errorcodes.h"
#include <QDataStream>
#include <QDebug>

namespace DFM {

// 静态变量：保存被挂起的任务URL
static QUrl s_workerUrlOnHold;

class SimpleJobPrivate {
public:
    SimpleJobPrivate()
        : worker(nullptr)
        , redirectionHandlingEnabled(true)
    {}

    QUrl url;                        // 任务URL
    Worker *worker;                  // 关联的Worker
    bool redirectionHandlingEnabled; // 是否启用重定向处理
};

SimpleJob::SimpleJob(const QUrl &url, QObject *parent)
    : Job(parent)
    , d(std::make_unique<SimpleJobPrivate>())
{
    d->url = url;
    qDebug() << "创建SimpleJob实例，URL:" << url;
}

SimpleJob::~SimpleJob()
{
    qDebug() << "销毁SimpleJob实例";
}

void SimpleJob::start()
{
    qDebug() << "SimpleJob::start()";
    // 基类中的虚函数实现，子类可以重写
}

void SimpleJob::cancel()
{
    qDebug() << "SimpleJob::cancel()";
    // 基类中的虚函数实现，子类可以重写
}

void SimpleJob::suspend()
{
    qDebug() << "SimpleJob::suspend()";
    // 基类中的虚函数实现，子类可以重写
}

void SimpleJob::resume()
{
    qDebug() << "SimpleJob::resume()";
    // 基类中的虚函数实现，子类可以重写
}

const QUrl &SimpleJob::url() const
{
    return d->url;
}

void SimpleJob::setUrl(const QUrl &url)
{
    d->url = url;
}

Worker *SimpleJob::worker() const
{
    return d->worker;
}

void SimpleJob::setWorker(Worker *worker)
{
    d->worker = worker;
}

void SimpleJob::putOnHold()
{
    qDebug() << "SimpleJob::putOnHold()";
    
    s_workerUrlOnHold = d->url;
}

void SimpleJob::removeOnHold()
{
    qDebug() << "SimpleJob::removeOnHold()";
    
    s_workerUrlOnHold = QUrl();
}

bool SimpleJob::isRedirectionHandlingEnabled() const
{
    return d->redirectionHandlingEnabled;
}

void SimpleJob::setRedirectionHandlingEnabled(bool handle)
{
    d->redirectionHandlingEnabled = handle;
}

void SimpleJob::slotError(int error, const QString &errorText)
{
    qDebug() << "SimpleJob::slotError(): 错误码:" << error << "错误文本:" << errorText;
    
    // 设置错误信息
    setError(error);
    setErrorText(errorText);
    
    // 发送错误信号
    Q_EMIT this->error(error, errorText);
    
    // 发送结果信号
    emitResult();
}

void SimpleJob::slotFinished()
{
    qDebug() << "SimpleJob::slotFinished()";
    
    // 发送结果信号
    emitResult();
}

void SimpleJob::slotWarning(const QString &warning)
{
    qDebug() << "SimpleJob::slotWarning(): 警告:" << warning;
    
    // 实际实现可能需要处理警告
}

void SimpleJob::slotMetaData(const QMap<QString, QString> &metaData)
{
    qDebug() << "SimpleJob::slotMetaData(): 元数据大小:" << metaData.size();
    
    // 实际实现可能需要处理元数据
}

// CustomSimpleJob 实现
CustomSimpleJob::CustomSimpleJob(const QUrl &url, QObject *parent)
    : SimpleJob(url, parent)
{
    qDebug() << "创建CustomSimpleJob实例，URL:" << url;
}

// 任务工厂函数实现
SimpleJob *file_delete(const QUrl &url, JobFlags flags)
{
    // 这些工厂函数应该在 Scheduler 中创建实际的任务
    // 使用 CustomSimpleJob 代替 SimpleJob
    CustomSimpleJob *job = new CustomSimpleJob(url);
    job->setFlags(flags);
    return job;
}

SimpleJob *rmdir(const QUrl &url)
{
    CustomSimpleJob *job = new CustomSimpleJob(url);
    return job;
}

SimpleJob *chmod(const QUrl &url, int permissions)
{
    CustomSimpleJob *job = new CustomSimpleJob(url);
    return job;
}

SimpleJob *chown(const QUrl &url, const QString &owner, const QString &group)
{
    CustomSimpleJob *job = new CustomSimpleJob(url);
    return job;
}

SimpleJob *setModificationTime(const QUrl &url, const QDateTime &mtime)
{
    CustomSimpleJob *job = new CustomSimpleJob(url);
    return job;
}

SimpleJob *rename(const QUrl &src, const QUrl &dest, JobFlags flags)
{
    CustomSimpleJob *job = new CustomSimpleJob(src);
    job->setFlags(flags);
    return job;
}

SimpleJob *symlink(const QString &target, const QUrl &dest, JobFlags flags)
{
    CustomSimpleJob *job = new CustomSimpleJob(dest);
    job->setFlags(flags);
    return job;
}

SimpleJob *special(const QUrl &url, const QByteArray &data, JobFlags flags)
{
    CustomSimpleJob *job = new CustomSimpleJob(url);
    job->setFlags(flags);
    return job;
}

SimpleJob *du(const QUrl &url, bool recursive, JobFlags flags)
{
    CustomSimpleJob *job = new CustomSimpleJob(url);
    job->setFlags(flags);
    return job;
}

} // namespace DFM 