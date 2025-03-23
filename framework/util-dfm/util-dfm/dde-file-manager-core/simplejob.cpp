#include "simplejob.h"
#include "worker.h"
#include "scheduler.h"
#include "commands.h"
#include <QDataStream>
#include <QDebug>

namespace DFM {

class SimpleJobPrivate {
public:
    SimpleJobPrivate(const QUrl &url)
        : m_url(url)
        , m_worker(nullptr)
        , m_redirectionHandlingEnabled(true)
    {}

    QUrl m_url;
    Worker *m_worker;
    bool m_redirectionHandlingEnabled;
};

SimpleJob::SimpleJob(const QUrl &url, QObject *parent)
    : Job(parent)
    , d(new SimpleJobPrivate(url))
{
    // 注册任务
    Scheduler::self()->registerJob(this, url);
}

SimpleJob::~SimpleJob()
{
    // 确保Worker不再引用此任务
    if (d->m_worker) {
        d->m_worker->setJob(nullptr);
    }
}

const QUrl &SimpleJob::url() const
{
    return d->m_url;
}

void SimpleJob::setUrl(const QUrl &url)
{
    d->m_url = url;
}

Worker *SimpleJob::worker() const
{
    return d->m_worker;
}

void SimpleJob::setWorker(Worker *worker)
{
    d->m_worker = worker;
}

void SimpleJob::putOnHold()
{
    if (d->m_worker) {
        Scheduler::self()->putWorkerOnHold(this, d->m_url);
    }
}

void SimpleJob::removeOnHold()
{
    Scheduler::self()->removeWorkerOnHold();
}

bool SimpleJob::isRedirectionHandlingEnabled() const
{
    return d->m_redirectionHandlingEnabled;
}

void SimpleJob::setRedirectionHandlingEnabled(bool handle)
{
    d->m_redirectionHandlingEnabled = handle;
}

void SimpleJob::slotError(int error, const QString &errorText)
{
    qDebug() << "SimpleJob::slotError:" << error << errorText;
    
    // 设置错误信息
    setError(error);
    setErrorText(errorText);
    
    // 结束任务
    emitResult();
}

void SimpleJob::slotFinished()
{
    qDebug() << "SimpleJob::slotFinished";
    
    // 告知调度器任务已完成
    if (d->m_worker) {
        Scheduler::self()->jobFinished(this, d->m_worker);
    }
    
    // 发送任务完成信号
    emitResult();
}

void SimpleJob::slotWarning(const QString &warning)
{
    qDebug() << "SimpleJob::slotWarning:" << warning;
    
    // 发送警告信号
    Q_EMIT warning(this, warning);
}

void SimpleJob::slotMetaData(const QMap<QString, QString> &metaData)
{
    // 处理元数据
    // 可以在子类中扩展此功能
}

// SimpleJob工厂函数
SimpleJob *file_delete(const QUrl &url, JobFlags flags)
{
    SimpleJob *job = new SimpleJob(url);
    
    // 设置任务标志
    job->setFlags(flags);
    
    // 发送删除文件命令
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    stream << CMD_DEL << url << true;
    
    job->worker()->send(CMD_DEL, packedArgs);
    
    return job;
}

SimpleJob *rmdir(const QUrl &url)
{
    SimpleJob *job = new SimpleJob(url);
    
    // 发送删除目录命令
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    stream << CMD_DEL << url << false;
    
    job->worker()->send(CMD_DEL, packedArgs);
    
    return job;
}

SimpleJob *chmod(const QUrl &url, int permissions)
{
    SimpleJob *job = new SimpleJob(url);
    
    // 发送更改权限命令
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    stream << CMD_CHMOD << url << permissions;
    
    job->worker()->send(CMD_CHMOD, packedArgs);
    
    return job;
}

SimpleJob *chown(const QUrl &url, const QString &owner, const QString &group)
{
    SimpleJob *job = new SimpleJob(url);
    
    // 发送更改所有者命令
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    stream << CMD_SPECIAL << url << owner << group;
    
    job->worker()->send(CMD_SPECIAL, packedArgs);
    
    return job;
}

SimpleJob *setModificationTime(const QUrl &url, const QDateTime &mtime)
{
    SimpleJob *job = new SimpleJob(url);
    
    // 发送设置修改时间命令
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    stream << CMD_SPECIAL << url << mtime;
    
    job->worker()->send(CMD_SPECIAL, packedArgs);
    
    return job;
}

SimpleJob *rename(const QUrl &src, const QUrl &dest, JobFlags flags)
{
    SimpleJob *job = new SimpleJob(src);
    
    // 设置任务标志
    job->setFlags(flags);
    
    // 发送重命名命令
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    stream << CMD_RENAME << src << dest << (int)flags;
    
    job->worker()->send(CMD_RENAME, packedArgs);
    
    return job;
}

SimpleJob *symlink(const QString &target, const QUrl &dest, JobFlags flags)
{
    SimpleJob *job = new SimpleJob(dest);
    
    // 设置任务标志
    job->setFlags(flags);
    
    // 发送创建符号链接命令
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    stream << CMD_SPECIAL << target << dest << (int)flags;
    
    job->worker()->send(CMD_SPECIAL, packedArgs);
    
    return job;
}

SimpleJob *special(const QUrl &url, const QByteArray &data, JobFlags flags)
{
    SimpleJob *job = new SimpleJob(url);
    
    // 设置任务标志
    job->setFlags(flags);
    
    // 发送特殊命令
    job->worker()->send(CMD_SPECIAL, data);
    
    return job;
}

SimpleJob *du(const QUrl &url, bool recursive, JobFlags flags)
{
    SimpleJob *job = new SimpleJob(url);
    
    // 设置任务标志
    job->setFlags(flags);
    
    // 发送磁盘使用统计命令
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    
    if (recursive) {
        stream << CMD_DU_RECURSIVE << url;
    } else {
        stream << CMD_DU << url;
    }
    
    job->worker()->send(CMD_SPECIAL, packedArgs);
    
    return job;
}

} // namespace DFM 