#include "workerinterface.h"
#include "connection.h"
#include "connection_p.h"
#include "scheduler.h"
#include "worker.h"
#include <QDebug>
#include <QUrl>
#include <QDataStream>
#include "commands.h"

namespace DFM {

// 定义信号，解决链接错误
void WorkerInterface::disconnected() {}
void WorkerInterface::data(const QByteArray &) {}
void WorkerInterface::dataReq() {}
void WorkerInterface::open() {}
void WorkerInterface::finished() {}
void WorkerInterface::statEntry(const QVariantMap &) {}
void WorkerInterface::listEntries(const QList<QVariantMap> &) {}
void WorkerInterface::canResume(qint64) {}
void WorkerInterface::error(int, const QString &) {}
void WorkerInterface::workerStatus(qint64, const QByteArray &, const QString &, bool) {}
void WorkerInterface::connected() {}
void WorkerInterface::written(qint64) {}
void WorkerInterface::totalSize(qint64) {}
void WorkerInterface::processedSize(qint64) {}
void WorkerInterface::position(qint64) {}
void WorkerInterface::truncated(qint64) {}
void WorkerInterface::speed(unsigned long) {}
void WorkerInterface::redirection(const QUrl &) {}
void WorkerInterface::mimeType(const QString &) {}
void WorkerInterface::warning(const QString &) {}
void WorkerInterface::infoMessage(const QString &) {}
void WorkerInterface::metaData(const MetaData &) {}

WorkerInterface::WorkerInterface(QObject *parent)
    : QObject(parent)
    , m_worker(nullptr)
    , m_job(nullptr)
{
    qDebug() << "创建WorkerInterface实例";
}

WorkerInterface::~WorkerInterface()
{
    qDebug() << "销毁WorkerInterface实例";
    
    // 不需要释放m_worker，因为它不是由我们创建的
}

void WorkerInterface::setJob(Job *job)
{
    m_job = job;
}

bool WorkerInterface::isConnected() const
{
    return m_worker != nullptr;
}

void WorkerInterface::connectWorker(const QString &protocol)
{
    qDebug() << "WorkerInterface::connectWorker(): 协议:" << protocol;
    
    // 获取Worker实例 - 简化实现
    int error = 0;
    QString errorText;
    QUrl url;
    url.setScheme(protocol);
    
    m_worker = Scheduler::self()->getWorker(protocol, url, error, errorText);
    
    if (!m_worker) {
        qWarning() << "WorkerInterface::connectWorker(): 无法获取Worker实例:" << errorText;
        return;
    }
}

void WorkerInterface::disconnectWorker()
{
    if (!m_worker) {
        return;
    }
    
    qDebug() << "WorkerInterface::disconnectWorker()";
    
    // 清空Worker指针
    m_worker = nullptr;
}

void WorkerInterface::send(int cmd, const QByteArray &data)
{
    if (!m_worker) {
        qWarning() << "WorkerInterface::send(): Worker未连接";
        return;
    }
    
    qDebug() << "WorkerInterface::send(): 命令:" << cmd;
    
    // 简化实现
}

void WorkerInterface::special(const QByteArray &data)
{
    if (!m_worker) {
        qWarning() << "WorkerInterface::special(): Worker未连接";
        return;
    }
    
    qDebug() << "WorkerInterface::special(): 数据大小:" << data.size();
}

bool WorkerInterface::dispatch()
{
    return false; // 临时实现
}

bool WorkerInterface::dispatch(int cmd, const QByteArray &data)
{
    return false; // 临时实现
}

void WorkerInterface::setOffset(qint64 o)
{
    // 临时实现
}

qint64 WorkerInterface::offset() const
{
    return 0; // 临时实现
}

void WorkerInterface::sendResumeAnswer(bool resume)
{
    // 临时实现
}

void WorkerInterface::sendMessageBoxAnswer(int result)
{
    // 临时实现
}

void WorkerInterface::slotWorkerDisconnected()
{
    qDebug() << "WorkerInterface::slotWorkerDisconnected()";
    
    // 处理Worker断开连接
    disconnectWorker();
    
    // 发出信号
    Q_EMIT disconnected();
}

void WorkerInterface::slotHostInfo(const QHostInfo &info)
{
    // 临时空实现
}

void WorkerInterface::calcSpeed()
{
    // 临时空实现
}

void WorkerInterface::messageBox(int type, const QString &text, const QString &title, 
                   const QString &primaryActionText, const QString &secondaryActionText)
{
    // 临时实现
}

void WorkerInterface::messageBox(int type, const QString &text, const QString &title,
                   const QString &primaryActionText, const QString &secondaryActionText,
                   const QString &dontAskAgainName)
{
    messageBox(type, text, title, primaryActionText, secondaryActionText);
}

} // namespace DFM 