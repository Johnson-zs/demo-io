#include "workerinterface.h"
#include "connection.h"
#include "connection_p.h"
#include "scheduler.h"
#include "worker.h"
#include <QDebug>
#include <QUrl>
#include <QDataStream>

namespace DFM {

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
    return m_worker && m_worker->isAlive();
}

void WorkerInterface::connectWorker(const QString &protocol)
{
    qDebug() << "WorkerInterface::connectWorker(): 协议:" << protocol;
    
    // 获取Worker实例
    m_worker = Scheduler::self()->getWorker(protocol);
    
    if (!m_worker) {
        qWarning() << "WorkerInterface::connectWorker(): 无法获取Worker实例";
        return;
    }
    
    // 连接信号
    connect(m_worker, &Worker::workerDisconnected, this, &WorkerInterface::slotWorkerDisconnected);
    
    // 增加引用计数
    m_worker->ref();
}

void WorkerInterface::disconnectWorker()
{
    if (!m_worker) {
        return;
    }
    
    qDebug() << "WorkerInterface::disconnectWorker()";
    
    // 断开信号
    disconnect(m_worker, &Worker::workerDisconnected, this, &WorkerInterface::slotWorkerDisconnected);
    
    // 减少引用计数
    m_worker->deref();
    
    // 清空Worker指针
    m_worker = nullptr;
}

void WorkerInterface::send(int cmd, const QByteArray &data)
{
    if (!m_worker || !m_worker->isAlive()) {
        qWarning() << "WorkerInterface::send(): Worker未连接";
        return;
    }
    
    qDebug() << "WorkerInterface::send(): 命令:" << cmd;
    
    // 发送命令
    m_worker->connection()->send(cmd, data);
}

void WorkerInterface::special(const QByteArray &data)
{
    if (!m_worker || !m_worker->isAlive()) {
        qWarning() << "WorkerInterface::special(): Worker未连接";
        return;
    }
    
    qDebug() << "WorkerInterface::special(): 数据大小:" << data.size();
    
    // 发送特殊命令
    send(CMD_SPECIAL, data);
}

void WorkerInterface::suspend()
{
    if (!m_worker || !m_worker->isAlive()) {
        qWarning() << "WorkerInterface::suspend(): Worker未连接";
        return;
    }
    
    qDebug() << "WorkerInterface::suspend()";
    
    // 发送挂起命令
    send(CMD_SUSPEND, QByteArray());
    
    // 挂起Worker
    m_worker->suspend();
}

void WorkerInterface::resume()
{
    if (!m_worker || !m_worker->isAlive()) {
        qWarning() << "WorkerInterface::resume(): Worker未连接";
        return;
    }
    
    qDebug() << "WorkerInterface::resume()";
    
    // 发送恢复命令
    send(CMD_RESUME, QByteArray());
    
    // 恢复Worker
    m_worker->resume();
}

void WorkerInterface::start(int cmd, const QUrl &url)
{
    if (!m_worker || !m_worker->isAlive()) {
        qWarning() << "WorkerInterface::start(): Worker未连接";
        return;
    }
    
    qDebug() << "WorkerInterface::start(): 命令:" << cmd << "URL:" << url;
    
    // 准备数据
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    
    stream << url;
    
    // 发送命令
    send(cmd, data);
    
    // 连接信号
    if (m_worker->connection()) {
        connect(m_worker->connection(), &Connection::commandReceived, this, &WorkerInterface::slotCommandReceived);
    }
}

void WorkerInterface::slotCommandReceived(int cmd)
{
    if (!m_worker || !m_worker->connection()) {
        return;
    }
    
    qDebug() << "WorkerInterface::slotCommandReceived(): 命令:" << cmd;
    
    // 读取数据
    Connection::Task task = m_worker->connection()->readCommand();
    if (task.cmd <= 0) {
        return;
    }
    
    // 处理预定义命令
    switch (task.cmd) {
        case CMD_ERROR:
            processError(task.data);
            break;
        case CMD_FINISHED:
            processFinished();
            break;
        case CMD_PROGRESS:
            processProgress(task.data);
            break;
        case CMD_MESSAGEBOX:
            processMessageBox(task.data);
            break;
        default:
            // 任务特定命令
            if (m_job) {
                m_job->processCommand(task.cmd, task.data);
            }
            break;
    }
}

void WorkerInterface::slotWorkerDisconnected(Worker *)
{
    qDebug() << "WorkerInterface::slotWorkerDisconnected()";
    
    // 清空Worker指针
    m_worker = nullptr;
    
    // 通知Job
    if (m_job) {
        m_job->workerDisconnected();
    }
    
    // 发送信号
    Q_EMIT disconnected();
}

void WorkerInterface::processError(const QByteArray &data)
{
    qDebug() << "WorkerInterface::processError()";
    
    QDataStream stream(data);
    int errCode;
    QString errMsg;
    
    stream >> errCode >> errMsg;
    
    qWarning() << "Worker错误:" << errCode << errMsg;
    
    // 通知Job
    if (m_job) {
        m_job->workerError(errCode, errMsg);
    }
    
    // 发送信号
    Q_EMIT error(errCode, errMsg);
}

void WorkerInterface::processFinished()
{
    qDebug() << "WorkerInterface::processFinished()";
    
    // 通知Job
    if (m_job) {
        m_job->workerFinished();
    }
    
    // 发送信号
    Q_EMIT finished();
}

void WorkerInterface::processProgress(const QByteArray &data)
{
    QDataStream stream(data);
    qulonglong processedSize;
    qulonglong totalSize;
    
    stream >> processedSize >> totalSize;
    
    qDebug() << "WorkerInterface::processProgress():" << processedSize << "/" << totalSize;
    
    // 通知Job
    if (m_job) {
        m_job->workerProgress(processedSize, totalSize);
    }
    
    // 发送信号
    Q_EMIT progress(processedSize, totalSize);
}

void WorkerInterface::processMessageBox(const QByteArray &data)
{
    QDataStream stream(data);
    QString text;
    QString title;
    QString primaryActionText;
    QString secondaryActionText;
    
    stream >> text >> title >> primaryActionText >> secondaryActionText;
    
    qDebug() << "WorkerInterface::processMessageBox():" << title << text;
    
    // 通知Job
    if (m_job) {
        m_job->workerMessageBox(text, title, primaryActionText, secondaryActionText);
    }
    
    // 发送信号
    Q_EMIT messageBox(text, title, primaryActionText, secondaryActionText);
}

} // namespace DFM 