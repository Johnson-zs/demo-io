#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QProcess>

#include <memory>
#include <optional>

namespace DFM {

// Worker 状态枚举
enum class WorkerState {
    None,
    Idle,
    Busy,
    Error,
    Dead
};

// Worker类型枚举
enum class WorkerType {
    Process,  // 运行在独立进程
    Thread    // 运行在线程内
};

// Worker 基类
class Worker : public QObject {
    Q_OBJECT
    Q_PROPERTY(WorkerState state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString protocol READ protocol CONSTANT)

public:
    explicit Worker(const QString &protocol, QObject *parent = nullptr);
    virtual ~Worker();

    // 工厂方法创建Worker实例
    static std::shared_ptr<Worker> createWorker(const QString &protocol, const QUrl &url, 
                                                QString &errorString);

    // 基本属性
    WorkerState state() const;
    QString protocol() const;
    QString workerPath() const;
    virtual WorkerType type() const = 0;

    // 状态查询
    bool isIdle() const;
    bool isBusy() const;
    bool isAlive() const;

    // 操作方法
    void setWorkerThread(class WorkerThread *thread);
    void setPID(qint64 pid);
    
    // 通信方法
    virtual bool sendCommand(int cmd, const QByteArray &data) = 0;
    virtual void terminate() = 0;

signals:
    void stateChanged(WorkerState state);
    void commandReceived(int cmd, const QByteArray &data);
    void error(const QString &errorString);
    void died();

protected:
    void setState(WorkerState state);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

// ProcessWorker - 进程内Worker实现
class ProcessWorker : public Worker {
    Q_OBJECT
public:
    explicit ProcessWorker(const QString &protocol, QObject *parent = nullptr);
    ~ProcessWorker() override;

    WorkerType type() const override { return WorkerType::Process; }
    
    bool sendCommand(int cmd, const QByteArray &data) override;
    void terminate() override;

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

// ThreadWorker - 线程内Worker实现
class ThreadWorker : public Worker {
    Q_OBJECT
public:
    explicit ThreadWorker(const QString &protocol, QObject *parent = nullptr);
    ~ThreadWorker() override;

    WorkerType type() const override { return WorkerType::Thread; }
    
    bool sendCommand(int cmd, const QByteArray &data) override;
    void terminate() override;
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace DFM 
