#include "worker.h"
#include "connection.h"

#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>
#include <QRandomGenerator>
#include <QStandardPaths>

namespace DFM {

class Worker::Private {
public:
    Private(const QString &protocol)
        : protocol(protocol)
    {
    }
    
    QString protocol;
    WorkerState state = WorkerState::None;
    qint64 pid = 0;
    class WorkerThread *thread = nullptr; // 仅用于线程模式
    QString workerPath;
};

Worker::Worker(const QString &protocol, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>(protocol))
{
}

Worker::~Worker()
{
    // 虚析构函数实现
}

std::shared_ptr<Worker> Worker::createWorker(const QString &protocol, const QUrl &url, QString &errorString)
{
    // 检查特殊协议
    if (protocol == QStringLiteral("file")) {
        // 文件协议使用线程模式
        auto worker = std::make_shared<ThreadWorker>(protocol);
        // TODO: 初始化文件协议线程
        return worker;
    } else {
        // 其他协议使用进程模式
        auto worker = std::make_shared<ProcessWorker>(protocol);
        
        // 查找worker库路径
        QString workerPlugin = QStringLiteral("libdfm-%1-worker.so").arg(protocol);
        QString workerPath = QStandardPaths::locate(QStandardPaths::LibraryLocation, 
                                                   QStringLiteral("dde-file-manager/workers/%1").arg(workerPlugin));
        
        if (workerPath.isEmpty()) {
            errorString = QStringLiteral("Worker plugin not found: %1").arg(workerPlugin);
            return nullptr;
        }
        
        worker->d->workerPath = workerPath;
        
        // 启动worker进程
        QString connectionName = QStringLiteral("dfm-worker-%1-%2")
                               .arg(protocol)
                               .arg(QRandomGenerator::global()->generate());
        
        QStringList args;
        args << QStringLiteral("--plugin") << workerPath
             << QStringLiteral("--connection") << connectionName;
        
        QProcess *process = new QProcess(worker.get());
        process->setProgram(QCoreApplication::applicationDirPath() + QStringLiteral("/dde-file-manager-worker"));
        process->setArguments(args);
        
        // 连接信号
        QObject::connect(process, &QProcess::errorOccurred, worker.get(), &ProcessWorker::processError);
        QObject::connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                       worker.get(), &ProcessWorker::processFinished);
                       
        // 启动进程
        process->start();
        if (!process->waitForStarted(5000)) {
            errorString = QStringLiteral("Failed to start worker process: %1").arg(process->errorString());
            return nullptr;
        }
        
        // 记录PID
        worker->setPID(process->processId());
        
        // 初始化连接
        // TODO: 建立与进程的连接
        
        return worker;
    }
}

WorkerState Worker::state() const
{
    return d->state;
}

QString Worker::protocol() const
{
    return d->protocol;
}

QString Worker::workerPath() const
{
    return d->workerPath;
}

bool Worker::isIdle() const
{
    return d->state == WorkerState::Idle;
}

bool Worker::isBusy() const
{
    return d->state == WorkerState::Busy;
}

bool Worker::isAlive() const
{
    return d->state != WorkerState::Dead && d->state != WorkerState::None;
}

void Worker::setWorkerThread(WorkerThread *thread)
{
    d->thread = thread;
}

void Worker::setPID(qint64 pid)
{
    d->pid = pid;
}

void Worker::setState(WorkerState state)
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state);
        
        if (state == WorkerState::Dead) {
            emit died();
        }
    }
}

// ProcessWorker Implementation
class ProcessWorker::Private {
public:
    QProcess *process = nullptr;
    std::unique_ptr<Connection> connection;
};

ProcessWorker::ProcessWorker(const QString &protocol, QObject *parent)
    : Worker(protocol, parent)
    , d(std::make_unique<Private>())
{
    setState(WorkerState::Idle);
}

ProcessWorker::~ProcessWorker()
{
    terminate();
}

bool ProcessWorker::sendCommand(int cmd, const QByteArray &data)
{
    if (d->connection && d->connection->isConnected()) {
        return d->connection->send(cmd, data);
    }
    return false;
}

void ProcessWorker::terminate()
{
    if (d->process) {
        d->process->terminate();
        if (!d->process->waitForFinished(3000)) {
            d->process->kill();
        }
    }
    
    if (d->connection) {
        d->connection->close();
    }
    
    setState(WorkerState::Dead);
}

void ProcessWorker::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Worker process finished with code:" << exitCode 
             << "status:" << exitStatus;
             
    if (exitStatus == QProcess::CrashExit) {
        emit error(QStringLiteral("Worker process crashed"));
    } else if (exitCode != 0) {
        emit error(QStringLiteral("Worker process exited with code %1").arg(exitCode));
    }
    
    setState(WorkerState::Dead);
}

void ProcessWorker::processError(QProcess::ProcessError error)
{
    QString errorString;
    switch (error) {
        case QProcess::FailedToStart:
            errorString = tr("Worker process failed to start");
            break;
        case QProcess::Crashed:
            errorString = tr("Worker process crashed");
            break;
        case QProcess::Timedout:
            errorString = tr("Worker process timed out");
            break;
        case QProcess::WriteError:
            errorString = tr("Failed to write to worker process");
            break;
        case QProcess::ReadError:
            errorString = tr("Failed to read from worker process");
            break;
        default:
            errorString = tr("Unknown worker process error");
            break;
    }
    
    emit this->error(errorString);
    setState(WorkerState::Dead);
}

// ThreadWorker Implementation
class ThreadWorker::Private {
public:
    // 线程相关成员
};

ThreadWorker::ThreadWorker(const QString &protocol, QObject *parent)
    : Worker(protocol, parent)
    , d(std::make_unique<Private>())
{
    setState(WorkerState::Idle);
}

ThreadWorker::~ThreadWorker()
{
    terminate();
}

bool ThreadWorker::sendCommand(int cmd, const QByteArray &data)
{
    // 线程模式下的命令传递
    // TODO: 实现线程间通信
    return false;
}

void ThreadWorker::terminate()
{
    // 终止线程
    // TODO: 实现线程终止
    setState(WorkerState::Dead);
}

} // namespace DFM 
